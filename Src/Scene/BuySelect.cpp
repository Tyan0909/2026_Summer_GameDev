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
static bool bs_leftStickYHold = false;

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

    // 既存画像読み込み
    itemImg_[0] = LoadGraph("Data/Image/BuySelect/NormalCamera.png");
    itemImg_[1] = LoadGraph("Data/Image/BuySelect/ZoomCamera.png");
    itemImg_[2] = LoadGraph("Data/Image/BuySelect/InsuranceCamera.png");
    itemImg_[3] = LoadGraph("Data/Image/BuySelect/Helmet.png");
    itemImg_[4] = LoadGraph("Data/Image/BuySelect/FragGrenade.png");
    itemImg_[5] = LoadGraph("Data/Image/BuySelect/SpikeTrap.png");
    itemImg_[6] = LoadGraph("Data/Image/BuySelect/ExplosiveTrap.png");

    items_.clear();
    // quantity 初期値 0
    items_.push_back({ "ノーマルカメラ", 0,   0, ITEM_TYPE::NORMAL_CAMERA });
    items_.push_back({ "ズームカメラ", 800, 0, ITEM_TYPE::ZOOM_CAMERA });
    items_.push_back({ "保険カメラ", 1200, 0, ITEM_TYPE::INSURANCE_CAMERA });
    items_.push_back({ "ヘルメット", 500,  0, ITEM_TYPE::HELMET });
    items_.push_back({ "フラググレネード", 300, 0, ITEM_TYPE::FRAG_GRENADE });
    items_.push_back({ "スパイクトラップ", 200, 0, ITEM_TYPE::SPIKE_TRAP });
    items_.push_back({ "爆発トラップ", 500,  0, ITEM_TYPE::EXPLOSIVE_TRAP });

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

    const auto padNo = InputManager::JOYPAD_NO::PAD1;
    const auto padState = ins.GetJPadInputState(padNo);
    const int stickThreshold = static_cast<int>(InputManager::AKEY_VAL_MAX * InputManager::THRESHOLD);

    bool isStickUp = false;
    bool isStickDown = false;

    if (!bs_leftStickYHold)
    {
        if (padState.AKeyLY <= -stickThreshold)
        {
            isStickUp = true;
            bs_leftStickYHold = true;
        }
        else if (padState.AKeyLY >= stickThreshold)
        {
            isStickDown = true;
            bs_leftStickYHold = true;
        }
    }
    else
    {
        if (padState.AKeyLY > -stickThreshold && padState.AKeyLY < stickThreshold)
        {
            bs_leftStickYHold = false;
        }
    }

    const bool isUp =
        ins.IsTrgDown(KEY_INPUT_UP) ||
        ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::D_PAD_UP) ||
        isStickUp;

    const bool isDown =
        ins.IsTrgDown(KEY_INPUT_DOWN) ||
        ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::D_PAD_DOWN) ||
        isStickDown;

    const bool isBuy =
        ins.IsTrgDown(KEY_INPUT_Z) ||
        ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::RIGHT);

    const bool isRemove =
        ins.IsTrgDown(KEY_INPUT_X) ||
        ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::DOWN);

    const bool isChangeScene =
        ins.IsTrgDown(KEY_INPUT_SPACE) ||
        ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::LEFT);

    if (isUp)
    {
        cursorIdx_ = (cursorIdx_ - 1 + (int)items_.size()) % (int)items_.size();
        if (bs_moveSE != -1) PlaySoundMem(bs_moveSE, DX_PLAYTYPE_BACK);
    }

    if (isDown)
    {
        cursorIdx_ = (cursorIdx_ + 1) % (int)items_.size();
        if (bs_moveSE != -1) PlaySoundMem(bs_moveSE, DX_PLAYTYPE_BACK);
    }

    if (isBuy)
    {
        int total = CalculateTotalPrice();
        const Item& it = items_[cursorIdx_];
        if (total + it.price <= currentAmount_)
        {
            items_[cursorIdx_].quantity += 1;
            if (bs_toggleSE != -1) PlaySoundMem(bs_toggleSE, DX_PLAYTYPE_BACK);
        }
    }

    if (isRemove)
    {
        if (items_[cursorIdx_].quantity > 0)
        {
            items_[cursorIdx_].quantity -= 1;
            if (bs_toggleSE != -1) PlaySoundMem(bs_toggleSE, DX_PLAYTYPE_BACK);
        }
    }

    if (isChangeScene)
    {
        int total = CalculateTotalPrice();

        int remaining = currentAmount_ - total;

        int carry = remaining - minAmount_;
        if (carry < 0) carry = 0;

        std::vector<int> purchased;
        for (const auto& it : items_)
        {
            for (int q = 0; q < it.quantity; ++q)
            {
                purchased.push_back(static_cast<int>(it.type));
            }
        }

        SceneManager::GetInstance().SetPurchasedItemTypes(purchased);

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
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
    DrawBox(listLeft, listTop, listRight, listBottom, GetColor(10, 30, 40), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
    DrawBox(listLeft, listTop, listRight, listBottom, GetColor(0, 200, 255), FALSE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 左側：アイテムリスト（フォントは中）
    int total = CalculateTotalPrice(); // ループの外に出して計算を1回に（最適化）

    for (int i = 0; i < (int)items_.size(); ++i)
    {
        bool canBuy = total + items_[i].price <= currentAmount_;

        // 1. デフォルトの文字色を「やわらかい水色」に
        unsigned int textColor = GetColor(160, 220, 240);
        if (items_[i].quantity == 0 && !canBuy)
        {
            textColor = GetColor(60, 90, 100); // 予算不足は「暗い青グレー」でグレーアウト
        }

        bool blink = ((GetNowCount() / 300) % 2) == 0;
        int rowX = listLeft + 20;
        int rowY = listTop + 14 + (i * 38);

        // 2. 選択中（カート入り）の行ハイライト（半透明のサイバー水色）
        if (items_[i].quantity > 0)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 40); // 薄く透けさせる（40/255）
            DrawBox(listLeft + 8, rowY - 4, listRight - 8, rowY + 24, GetColor(0, 200, 255), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            // 数量を右寄せで表示
            char qbuf[16];
            snprintf(qbuf, sizeof(qbuf), "x%d", items_[i].quantity);
            DrawFormatStringToHandle(listRight - 56, rowY, GetColor(0, 255, 200), fontMid_, "%s", qbuf);
        }

        // 3. カーソル位置（フォーカス）の処理
        if (i == cursorIdx_)
        {
            const char* cursorMark = blink ? "▶" : " ";
            DrawFormatStringToHandle(rowX - 28, rowY, GetColor(0, 255, 255), fontMid_, "%s", cursorMark);
            textColor = GetColor(255, 255, 255); // 白
        }

        // 4. アイテム名と価格の描画
        DrawFormatStringToHandle(rowX, rowY, textColor, fontMid_, "%s : %d G", items_[i].name.c_str(), items_[i].price);

        // 操作ガイド
        if (i == cursorIdx_)
        {
            DrawFormatStringToHandle(rowX, rowY + 20, GetColor(180, 255, 255), fontSmall_, "B:購入  A:削除 X:画面遷移");
		}

        // 移動方法
        if (i == cursorIdx_)
        {
            DrawFormatStringToHandle(rowX + 200, rowY + 20, GetColor(180, 255, 255), fontSmall_, "左↑↓:選択");
		}

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

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawBox(statusBoxX, statusBoxY, statusBoxX + statusBoxW, statusBoxY + statusBoxH, GetColor(10, 24, 32), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        unsigned int neonCyan = GetColor(0, 200, 255);

        DrawBox(statusBoxX, statusBoxY, statusBoxX + statusBoxW, statusBoxY + statusBoxH, neonCyan, FALSE);
        DrawBox(statusBoxX - 1, statusBoxY - 1, statusBoxX + 14, statusBoxY + 6, neonCyan, TRUE);
        DrawBox(statusBoxX + statusBoxW - 14, statusBoxY - 1, statusBoxX + statusBoxW + 1, statusBoxY + 6, neonCyan, TRUE);

        unsigned int goldColor = GetColor(255, 230, 100); // 所持金
        unsigned int cartColor = GetColor(0, 255, 220);   // カート
        unsigned int remainColor = GetColor(180, 230, 245); // 残り

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

    // 右側プレビュー領域（status の下） — 描画は変わらず
    {
        const int winLeft = 700;
        const int winTop = 26 + 120 + 22;
        const int winRight = 1150;
        const int winBottom = winTop + 240;

        int previewIdx = cursorIdx_;
        if (previewIdx < 0) previewIdx = 0;
        if (previewIdx > (int)items_.size() - 1) previewIdx = (int)items_.size() - 1;
        const Item& currentItem = items_[previewIdx];

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawBox(winLeft, winTop, winRight, winBottom, GetColor(10, 24, 32), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
        DrawBox(winLeft, winTop, winRight, winBottom, GetColor(0, 200, 255), FALSE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        DrawLine(winLeft, winTop, winLeft + 10, winTop, GetColor(0, 255, 255), 2);
        DrawLine(winLeft, winTop, winLeft, winTop + 10, GetColor(0, 255, 255), 2);
        DrawLine(winRight, winTop, winRight - 10, winTop, GetColor(0, 255, 255), 2);
        DrawLine(winRight, winTop, winRight, winTop + 10, GetColor(0, 255, 255), 2);

        unsigned int nameColor = GetColor(255, 255, 255);
        if (fontMid_ != -1)
            DrawFormatStringToHandle(winLeft + 24, winTop + 12, nameColor, fontMid_, "%s", currentItem.name.c_str());
        else
            DrawFormatString(winLeft + 24, winTop + 12, nameColor, "%s", currentItem.name.c_str());

        const int areaW = 380;
        const int areaH = 140;
        const int centerX = (winLeft + winRight) / 2;
        const int bgLeft = centerX - areaW / 2;
        const int bgTop = winTop + 44;
        const int bgRight = bgLeft + areaW;
        const int bgBottom = bgTop + areaH;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
        DrawBox(bgLeft, bgTop, bgRight, bgBottom, GetColor(0, 180, 255), FALSE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

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
                DrawRotaGraph(centerX, centerY, scale, 0.0, imgHandle, TRUE);
            }
        }

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
        if (item.quantity > 0) total += item.price * item.quantity;
    return total;
}

void BuySelect::ToggleItemSelection(int idx)
{
    // 互換用: Z=増加、X=減少 を使うためここは簡易 no-op
    (void)idx;
}