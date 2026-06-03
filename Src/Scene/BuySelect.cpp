#define NOMINMAX
#include <DxLib.h>
#include <algorithm> // std::min
#include <sstream>   // std::istringstream（必要なら使うが今回は単純描画に変更）
#include "BuySelect.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"

#ifndef DX_FONTTYPE_ANTIALIAS
// 環境によって定義がない場合は安全なデフォルトを用意
#define DX_FONTTYPE_ANTIALIAS 0
#endif

// ファイルローカルの効果音ハンドル
static int bs_moveSE = -1;
static int bs_toggleSE = -1;
static int bs_confirmSE = -1;

BuySelect::BuySelect(void) {}
BuySelect::~BuySelect(void) {}

void BuySelect::Init(void)
{
    // 背景画像を読み込む（ファイルを Data/Image/BuySelect/Background.png に置いてください）
    bgHandle_ = LoadGraph("Data/Image/BuySelect/Background.png");

    // 効果音読み込み
    bs_moveSE = LoadSoundMem("Data/Sound/move.mp3");
    bs_toggleSE = LoadSoundMem("Data/Sound/toggle.mp3");
    bs_confirmSE = LoadSoundMem("Data/Sound/confirm.mp3");

    // フォント作成（日本語フォントは環境に依存します。必要ならフォント名を指定）
    fontLarge_ = CreateFontToHandle(NULL, 28, -1, DX_FONTTYPE_ANTIALIAS);
    fontMid_ = CreateFontToHandle(NULL, 20, -1, DX_FONTTYPE_ANTIALIAS);
    fontSmall_ = CreateFontToHandle(NULL, 16, -1, DX_FONTTYPE_ANTIALIAS);

    // 既存画像読み込みに罠画像を追加
    itemImg_[0] = LoadGraph("Data/Image/BuySelect/NormalCamera.png");
    itemImg_[1] = LoadGraph("Data/Image/BuySelect/ZoomCamera.png");
    itemImg_[2] = LoadGraph("Data/Image/BuySelect/InsuranceCamera.png");
    itemImg_[3] = LoadGraph("Data/Image/BuySelect/Helmet.png");
    itemImg_[4] = LoadGraph("Data/Image/BuySelect/FragGrenade.png");
    itemImg_[5] = LoadGraph("Data/Image/BuySelect/SpikeTrap.png");
    itemImg_[6] = LoadGraph("Data/Image/BuySelect/ExplosiveTrap.png");

    items_.clear();
    items_.push_back({ "ノーマルカメラ", 0,false,ITEM_TYPE::NORMAL_CAMERA });
    items_.push_back({ "ズームカメラ", 800, false,ITEM_TYPE::ZOOM_CAMERA });
    items_.push_back({ "保険カメラ", 1200, false,ITEM_TYPE::INSURANCE_CAMERA });
    items_.push_back({ "ヘルメット", 500, false,ITEM_TYPE::HELMET });
    items_.push_back({ "フラググレネード", 300, false,ITEM_TYPE::FRAG_GRENADE });
    items_.push_back({ "スパイクトラップ", 200, false, ITEM_TYPE::SPIKE_TRAP });
    items_.push_back({ "爆発トラップ", 600, false, ITEM_TYPE::EXPLOSIVE_TRAP });

    // SceneManager に保存されている「最低保証を除いた持ち越し分」を取得
    int carry = SceneManager::GetInstance().GetCarryMoney();

    // 所持金 = 最低保証 + carry
    currentAmount_ = minAmount_ + carry;

    cursorIdx_ = 0;
}

void BuySelect::Update(void)
{
    InputManager& ins = InputManager::GetInstance();
    SceneManager& scene = SceneManager::GetInstance();

    if (items_.empty()) return;

    // 上下カーソル移動
    if (ins.IsTrgDown(KEY_INPUT_UP))
    {
        cursorIdx_ = (cursorIdx_ - 1 + (int)items_.size()) % (int)items_.size();
        if (bs_moveSE != -1) PlaySoundMem(bs_moveSE, DX_PLAYTYPE_BACK);
    }
    if (ins.IsTrgDown(KEY_INPUT_DOWN))
    {
        cursorIdx_ = (cursorIdx_ + 1) % (int)items_.size();
        if (bs_moveSE != -1) PlaySoundMem(bs_moveSE, DX_PLAYTYPE_BACK);
    }

    // Zキーで選択トグル
    if (ins.IsTrgDown(KEY_INPUT_Z))
    {
        ToggleItemSelection(cursorIdx_);
        if (bs_toggleSE != -1) PlaySoundMem(bs_toggleSE, DX_PLAYTYPE_BACK);
    }

    // SPACEキーで購入確定（清算）
    if (ins.IsTrgDown(KEY_INPUT_SPACE))
    {
        int total = CalculateTotalPrice();

        // 購入後の所持金（最低保証を含む）
        int remaining = currentAmount_ - total;

        int carry = remaining - minAmount_;
        if (carry < 0) carry = 0;

        scene.SetCarryMoney(carry);
        if (bs_confirmSE != -1) PlaySoundMem(bs_confirmSE, DX_PLAYTYPE_BACK);
        scene.ChangeScene(SceneManager::SCENE_ID::LOADING);
    }
}

void BuySelect::Draw(void)
{
    // 画面サイズ取得（配置位置計算で使用）
    int screenW = 0, screenH = 0;
    GetDrawScreenSize(&screenW, &screenH);

    // 背景描画（伸縮して画面全体に描画）
    if (bgHandle_ != -1)
    {
        DrawExtendGraph(0, 0, screenW, screenH, bgHandle_, FALSE);
    }
    else
    {
        // 背景画像が無い場合は暗めのグラデーション代替（シンプル）
        DrawBox(0, 0, screenW, screenH, GetColor(12, 18, 30), TRUE);
    }

    // タイトル（大フォント、少し左上）
    if (fontLarge_ != -1)
    {
        DrawFormatStringToHandle(100, 40, GetColor(230, 230, 230), fontLarge_, "アイテム選択・購入");
    }
    else
    {
        DrawString(100, 50, "アイテム選択・購入", GetColor(255, 255, 255));
    }

    // 左側パネルの背景（半透明座布団）
    const int listLeft = 160;
    const int listTop = 100;
    const int listRight = 560;
    const int listBottom = screenH - 80;
    // 1. ブレンドモードを「アルファブレンド（半透明）」に設定し、不透明度を「120」にする（0〜255で指定）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
    // 2. ウィンドウの背景（座布団）を半透明で描画（暗い水色にするのが近未来風のコツ！）
    DrawBox(listLeft, listTop, listRight, listBottom, GetColor(10, 30, 40), TRUE);
    // 3. ブレンドモードを「不透明度 255（くっきり）」に戻す
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
    // 4. 外枠をくっきりとした鮮やかな水色で描画
    DrawBox(listLeft, listTop, listRight, listBottom, GetColor(0, 200, 255), FALSE);
    // 5. 最後にブレンドモードを通常（NOBLEND）に戻しておく（他への影響を防ぐため）
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 左側：アイテムリスト（フォントは中）
    int total = CalculateTotalPrice(); // ループの外に出して計算を1回に（最適化）

    for (int i = 0; i < (int)items_.size(); ++i)
    {
        bool canBuy = total + items_[i].price <= currentAmount_;

        // 1. デフォルトの文字色を「やわらかい水色」に
        unsigned int textColor = GetColor(160, 220, 240);
        if (!items_[i].isSelected && !canBuy)
        {
            textColor = GetColor(60, 90, 100); // 予算不足は「暗い青グレー」でグレーアウト
        }

        bool blink = ((GetNowCount() / 300) % 2) == 0;
        int rowX = listLeft + 20;
        int rowY = listTop + 14 + (i * 38);

        // 2. 選択中（カート入り）の行ハイライト（半透明のサイバー水色）
        if (items_[i].isSelected)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 40); // 薄く透けさせる（40/255）
            DrawBox(listLeft + 8, rowY - 4, listRight - 8, rowY + 24, GetColor(0, 200, 255), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            // チェックマークも水色（ネオン）に
            DrawFormatStringToHandle(listRight - 40, rowY, GetColor(0, 255, 200), fontMid_, "✓");
        }

        // 3. カーソル位置（フォーカス）の処理
        if (i == cursorIdx_)
        {
            // カーソルアイコンは常時表示か、点滅させる（常時表示の方が視認性は良いです）
            const char* cursorMark = blink ? "▶" : " ";
            DrawFormatStringToHandle(rowX - 28, rowY, GetColor(0, 255, 255), fontMid_, "%s", cursorMark);

            // 【重要】文字色は点滅させず、常時「一番明るい発光水色」に固定
            textColor = GetColor(255, 255, 255); // 白、または GetColor(120, 255, 240)
        }

        // 4. アイテム名と価格の描画
        // カートに入っている（isSelected）かつ、カーソルが合っていない時は文字色を少し変えても綺麗です
        if (items_[i].isSelected && i != cursorIdx_)
        {
            textColor = GetColor(0, 255, 200); // カート内アイテム用の緑がかった水色
        }

        DrawFormatStringToHandle(rowX, rowY, textColor, fontMid_, "%s : %d G", items_[i].name.c_str(), items_[i].price);
    }

    // 経済情報（右上のステータスボックス、フォントは中）
    const int statusBoxW = 340;
    const int statusBoxH = 120;
    const int statusBoxX = screenW - statusBoxW - 36;
    const int statusBoxY = 26;

    {
        int total = CalculateTotalPrice();
        int remaining = currentAmount_ - total;
        if (remaining < 0) remaining = 0;

        // 1. 背景座布団（アルファ値を100に下げ、完全に暗い青緑に統一）
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawBox(statusBoxX, statusBoxY, statusBoxX + statusBoxW, statusBoxY + statusBoxH, GetColor(10, 24, 32), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // サイバーネオン水色の定義（使い回せるように変数化しておくと便利です）
        unsigned int neonCyan = GetColor(0, 200, 255);

        // 2. 枠線 + 小装飾（すべて鮮やかな水色に統一）
        DrawBox(statusBoxX, statusBoxY, statusBoxX + statusBoxW, statusBoxY + statusBoxH, neonCyan, FALSE);

        // 上の突起装飾も水色で塗りつぶし
        DrawBox(statusBoxX - 1, statusBoxY - 1, statusBoxX + 14, statusBoxY + 6, neonCyan, TRUE);
        DrawBox(statusBoxX + statusBoxW - 14, statusBoxY - 1, statusBoxX + statusBoxW + 1, statusBoxY + 6, neonCyan, TRUE);

        // 3. テキスト色の調整（黄色やグレーを廃止し、白と水色のグラデーションに）
        unsigned int goldColor = GetColor(255, 230, 100); // 所持金：2枚目に合わせた少し白みの強いゴールド
        unsigned int cartColor = GetColor(0, 255, 220);   // カート：発光する明るい水色
        unsigned int remainColor = GetColor(180, 230, 245); // 残り：視認性の高いやわらかい水色

        if (fontMid_ != -1)
        {
            DrawFormatStringToHandle(statusBoxX + 14, statusBoxY + 12, goldColor, fontMid_, "所持金: %d G", currentAmount_);
            DrawFormatStringToHandle(statusBoxX + 14, statusBoxY + 40, cartColor, fontMid_, "カート合計: %d G", total);
            DrawFormatStringToHandle(statusBoxX + 14, statusBoxY + 68, remainColor, fontMid_, "購入後の残り: %d G", remaining);
        }
        else
        {
            DrawFormatString(statusBoxX + 12, statusBoxY + 8, goldColor, "所持金: %d G", currentAmount_);
            DrawFormatString(statusBoxX + 12, statusBoxY + 38, cartColor, "カート合計: %d G", total);
            DrawFormatString(statusBoxX + 12, statusBoxY + 62, remainColor, "購入後の残り: %d G", remaining);
        }
    }

    // 右側プレビュー領域（ステータスボックスの下に配置）
    {
        const int winLeft = 700;
        const int winTop = statusBoxY + statusBoxH + 22; // statusBox の下
        const int winRight = 1150;
        const int winBottom = winTop + 240;

        int previewIdx = cursorIdx_;
        if (previewIdx < 0) previewIdx = 0;
        if (previewIdx > (int)items_.size() - 1) previewIdx = (int)items_.size() - 1;
        const Item& currentItem = items_[previewIdx];

        // 1. 背景座布団（アルファ値を下げて、暗い青緑に変えることで近未来のガラスパネル風に）
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100); // 200から100に下げて透明度アップ
        DrawBox(winLeft, winTop, winRight, winBottom, GetColor(10, 24, 32), TRUE);

        // 2. 外枠を鮮やかな水色にする
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255); // くっきり表示
        DrawBox(winLeft, winTop, winRight, winBottom, GetColor(0, 200, 255), FALSE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // 四隅に小さなL字の飾りを付けると、さらに2枚目の見た目に近づきます（お好みで！）
        DrawLine(winLeft, winTop, winLeft + 10, winTop, GetColor(0, 255, 255), 2);
        DrawLine(winLeft, winTop, winLeft, winTop + 10, GetColor(0, 255, 255), 2);
        DrawLine(winRight, winTop, winRight - 10, winTop, GetColor(0, 255, 255), 2);
        DrawLine(winRight, winTop, winRight, winTop + 10, GetColor(0, 255, 255), 2);

        // アイテム名（上：ここもネオン水色、またはカーソルと同じ白に）
        unsigned int nameColor = GetColor(255, 255, 255); // 白、もしくは GetColor(0, 255, 255)
        if (fontMid_ != -1)
            DrawFormatStringToHandle(winLeft + 24, winTop + 12, nameColor, fontMid_, "%s", currentItem.name.c_str());
        else
            DrawFormatString(winLeft + 24, winTop + 12, nameColor, "%s", currentItem.name.c_str());

        // 3. 画像用背景の調整
        // 2枚目の画像のように、背後のガレージを活かすために「不透明なグレーの箱」は描画せず、
        // 代わりに「画像を囲む薄い水色のスキャンライン（枠）」を描画すると近未来感が出ます。
        const int areaW = 380;
        const int areaH = 140;
        const int centerX = (winLeft + winRight) / 2;
        const int bgLeft = centerX - areaW / 2;
        const int bgTop = winTop + 44;
        const int bgRight = bgLeft + areaW;
        const int bgBottom = bgTop + areaH;

        // グレーのベタ塗りをやめ、薄い水色の枠線にする
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
        DrawBox(bgLeft, bgTop, bgRight, bgBottom, GetColor(0, 180, 255), FALSE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // 画像を中央に描画（フィット）
        int imgHandle = itemImg_[(int)currentItem.type];
        if (imgHandle != -1)
        {
            int imgW = 0, imgH = 0;
            GetGraphSize(imgHandle, &imgW, &imgH);
            if (imgW > 0 && imgH > 0)
            {
                float scale = std::min(areaW / (float)imgW, areaH / (float)imgH);
                if (scale > 1.0f) scale = 1.0f;
                int centerY = bgTop + areaH / 2;

                // アイコン画像自体が背景付き（トランシーバーの画像のような薄茶背景）の場合、
                // DXLib側で透過処理（TRUE）が有効になっていれば、綺麗に浮かび上がります。
                DrawRotaGraph(centerX, centerY, scale, 0.0, imgHandle, TRUE);
            }
        }

        // 説明文（画像下：こちらもやわらかい水色・白ベースに）
        const char* desc = "";
        switch (currentItem.type)
        {
        case ITEM_TYPE::NORMAL_CAMERA:
            desc = "標準的なカメラ。倍率の変更はできないが、基本性能に優れる。";
            break;
        case ITEM_TYPE::ZOOM_CAMERA:
            desc = "遠距離ズーム撮影が可能。遠い被写体を狙いやすい。";
            break;
        case ITEM_TYPE::INSURANCE_CAMERA:
            desc = "死亡してもスコアを保持する。リスクヘッジ用。";
            break;
        case ITEM_TYPE::HELMET:
            desc = "被ダメージを軽減する装備。生存率アップ。";
            break;
        case ITEM_TYPE::FRAG_GRENADE:
            desc = "敵を吹き飛ばす爆弾。近距離で高ダメージ。";
            break;
        case ITEM_TYPE::SPIKE_TRAP:
            desc = "床に設置して敵を足止めする罠（通行中ダメージ）。";
            break;
        case ITEM_TYPE::EXPLOSIVE_TRAP:
            desc = "衝撃で爆発する罠（近くの敵に大ダメージ）。";
            break;
        default:
            desc = "";
            break;
        }

        unsigned int descColor = GetColor(180, 230, 245); // やわらかい水色
        if (fontSmall_ != -1)
            DrawFormatStringToHandle(winLeft + 24, bgBottom + 14, descColor, fontSmall_, "%s", desc);
        else
            DrawFormatString(winLeft + 24, bgBottom + 14, descColor, "%s", desc);
    }
}

void BuySelect::Release(void)
{
    items_.clear();
    // 背景・画像解放
    if (bgHandle_ != -1)
    {
        DeleteGraph(bgHandle_);
        bgHandle_ = -1;
    }
    for (int i = 0; i < 7; ++i)
    {
        if (itemImg_[i] != -1)
        {
            DeleteGraph(itemImg_[i]);
            itemImg_[i] = -1;
        }
    }
    if (fontLarge_ != -1) { DeleteFontToHandle(fontLarge_); fontLarge_ = -1; }
    if (fontMid_ != -1) { DeleteFontToHandle(fontMid_); fontMid_ = -1; }
    if (fontSmall_ != -1) { DeleteFontToHandle(fontSmall_); fontSmall_ = -1; }

    // 効果音解放
    if (bs_moveSE != -1) { DeleteSoundMem(bs_moveSE); bs_moveSE = -1; }
    if (bs_toggleSE != -1) { DeleteSoundMem(bs_toggleSE); bs_toggleSE = -1; }
    if (bs_confirmSE != -1) { DeleteSoundMem(bs_confirmSE); bs_confirmSE = -1; }
}

int BuySelect::CalculateTotalPrice() const
{
    int total = 0;
    for (const auto& item : items_)
        if (item.isSelected) total += item.price;
    return total;
}

void BuySelect::ToggleItemSelection(int idx)
{
    if (idx < 0 || idx >= (int)items_.size()) return;

    if (items_[idx].isSelected)
    {
        items_[idx].isSelected = false;
    }
    else
    {
        // 所持金を超えない範囲でのみ選択可能
        if (CalculateTotalPrice() + items_[idx].price <= currentAmount_)
        {
            items_[idx].isSelected = true;
        }
    }
}