#define NOMINMAX
#include <DxLib.h>
#include <algorithm> // std::min
#include <sstream>   // std::istringstream（必要なら使うが今回は単純描画に変更）
#include "BuySelect.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/SoundManager.h"

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

    // 既存画像読み込み
    itemImg_[0] = LoadGraph("Data/Image/BuySelect/NormalCamera.png");
    itemImg_[1] = LoadGraph("Data/Image/BuySelect/ZoomCamera.png");
    itemImg_[2] = LoadGraph("Data/Image/BuySelect/InsuranceCamera.png");
    itemImg_[3] = LoadGraph("Data/Image/BuySelect/Helmet.png");
    itemImg_[4] = LoadGraph("Data/Image/BuySelect/FragGrenade.png");
    itemImg_[5] = LoadGraph("Data/Image/BuySelect/SpikeTrap.png");
    itemImg_[6] = LoadGraph("Data/Image/BuySelect/ExplosiveTrap.png");
    messageFaceHandle_ =
        LoadGraph("Data/Image/BuySelect/PlayerText.png");

	// 購入メッセージ用フォント
    messageFont_ = CreateFontToHandle(
        NULL,
        28,                         // ← 好きなサイズ
        -1,
        DX_FONTTYPE_ANTIALIAS);

	// 名前表示用フォント
    int nameFont_ = CreateFontToHandle(
        NULL,
        22,
        3,
        DX_FONTTYPE_ANTIALIAS);


    /*printfDx("nameFont = %d\n", nameFont_);*/

    playerItems_.clear();

    int playerNum =
        SceneManager::GetInstance().GetPlayerNum();

    purchasedItemsPerPlayer_.clear();
    purchasedItemsPerPlayer_.resize(playerNum);

    playerItems_.resize(playerNum);

	// 各プレイヤーの購入リストを初期化
    for (int i = 0; i < playerNum; i++)
    {
        playerItems_[i] =
        {
            { "ノーマルカメラ", 0,    0, ITEM_TYPE::NORMAL_CAMERA },
            { "ズームカメラ", 800,  0, ITEM_TYPE::ZOOM_CAMERA },
            { "保険カメラ", 1200, 0, ITEM_TYPE::INSURANCE_CAMERA },
            { "ヘルメット", 500,  0, ITEM_TYPE::HELMET },
            { "フラググレネード", 300, 0, ITEM_TYPE::FRAG_GRENADE },
            { "スパイクトラップ", 200, 0, ITEM_TYPE::SPIKE_TRAP },
            { "爆発トラップ", 500,  0, ITEM_TYPE::EXPLOSIVE_TRAP }
        };
    }


    const auto& money =
        SceneManager::GetInstance().GetPlayerMoney();

    playerMoney_.assign(playerNum, 2000);

    for (int i = 0; i < std::min(playerNum, (int)money.size()); i++)
    {
        playerMoney_[i] = money[i];
    }

    cursorIdx_ = 0;
    isTurnChange_ = true;
    turnChangeFrame_ = 0;
    currentPlayer_ = 0;
}

void BuySelect::Update(void)
{

    if (isTurnChange_)
    {
        turnChangeFrame_++;

        // SPACEまたは2秒経過で開始
        if (InputManager::GetInstance().IsTrgDown(KEY_INPUT_SPACE) ||
            turnChangeFrame_ >= TURN_CHANGE_TIME)
        {
            printfDx("Start Player %d\n", currentPlayer_ + 1);
            isTurnChange_ = false;
        }

        return;
    }

    if (currentPlayer_ >= playerItems_.size())
    {
        return;
    }

    InputManager& ins = InputManager::GetInstance();
    SceneManager& scene = SceneManager::GetInstance();

    auto& items =
        playerItems_[currentPlayer_];

    if (items.empty()) return;

    // 上下カーソル移動
    if (ins.IsTrgDown(KEY_INPUT_UP))
    {
        cursorIdx_ = (cursorIdx_ - 1 + (int)items.size()) % (int)items.size();
        if (bs_moveSE != -1) PlaySoundMem(bs_moveSE, DX_PLAYTYPE_BACK);
    }
    if (ins.IsTrgDown(KEY_INPUT_DOWN))
    {
        cursorIdx_ =(cursorIdx_ + 1) %(int)items.size();
        if (bs_moveSE != -1) PlaySoundMem(bs_moveSE, DX_PLAYTYPE_BACK);
    }

    // Zキーで数量を +1 (購入を増やす)
    if (ins.IsTrgDown(KEY_INPUT_Z))
    {
        Item& it = items[cursorIdx_];


        // カメラ購入制限
        if (IsCameraItem(it.type))
        {
            // 同じカメラを2個以上買おうとした場合
            if (it.quantity >= 1)
            {
                buyMessage_ = "カメラは1個まで購入できない";
                buyMessageFrame_ = 120;
                return;
            }


            // 他のカメラがカートに入っているか確認
            bool alreadyCamera = false;

            for (const auto& item : items)
            {
                if (IsCameraItem(item.type) &&
                    item.quantity > 0)
                {
                    alreadyCamera = true;
                    break;
                }
            }


            if (alreadyCamera)
            {
                buyMessage_ = "カメラは1種類のみ選択できるぞ！";
                buyMessageFrame_ = 120;
                return;
            }
        }


        int total = CalculateTotalPrice();


        if (total + it.price <= playerMoney_[currentPlayer_])
        {
            it.quantity += 1;

            if (bs_toggleSE != -1)
            {
                PlaySoundMem(
                    bs_toggleSE,
                    DX_PLAYTYPE_BACK);
            }
        }
        else
        {
            buyMessage_ = "所持金が足りないぞ";
            buyMessageFrame_ = 120;
        }
    }

    // Xキーで数量を -1 (購入数を減らす)
    if (ins.IsTrgDown(KEY_INPUT_X))
    {
        if (items[cursorIdx_].quantity > 0)
        {
            items[cursorIdx_].quantity -= 1;
            if (bs_toggleSE != -1) PlaySoundMem(bs_toggleSE, DX_PLAYTYPE_BACK);
        }
    }

    // SPACEキーで購入確定（清算）
    if (ins.IsTrgDown(KEY_INPUT_SPACE))
    {
 
		// 現在のプレイヤーの購入リストを保存
        std::vector<int> purchased;

        bool hasCamera = false;

        // すでにカメラ購入済みか確認
        for (const auto& item : purchasedItemsPerPlayer_[currentPlayer_])
        {
            if (IsCameraItem(static_cast<ITEM_TYPE>(item)))
            {
                hasCamera = true;
                break;
            }
        }


        for (const auto& it : items)
        {
            if (it.quantity <= 0)
                continue;


            // カメラの場合
            if (IsCameraItem(it.type))
            {
                // すでにカメラがある場合は追加しない
                if (hasCamera)
                {
                    continue;
                }

                // カメラ購入済みにする
                hasCamera = true;
            }


            for (int q = 0; q < it.quantity; q++)
            {
                purchased.push_back(
                    static_cast<int>(it.type));
            }
        }

        purchasedItemsPerPlayer_[currentPlayer_] = purchased;

        int grandTotal = CalculateTotalPrice();

        playerMoney_[currentPlayer_] -= grandTotal;

        if (playerMoney_[currentPlayer_] < minAmount_)
        {
            playerMoney_[currentPlayer_] = minAmount_;
        }

        previousPlayer_ = currentPlayer_;
        currentPlayer_++;


        if (currentPlayer_ <
            SceneManager::GetInstance().GetPlayerNum())
        {
            cursorIdx_ = 0;

            // 全数量リセット
            for (auto& item : playerItems_[currentPlayer_])
            {
                item.quantity = 0;
            }

            isTurnChange_ = true;
            turnChangeFrame_ = 0;

            if (bs_confirmSE != -1)
            {
                PlaySoundMem(
                    bs_confirmSE,
                    DX_PLAYTYPE_BACK);
            }

            return;
        }

        bool hasInsurance = false;

        for (const auto& playerItems : playerItems_)
        {
            for (const auto& item : playerItems)
            {
                if (item.type == ITEM_TYPE::INSURANCE_CAMERA &&
                    item.quantity > 0)
                {
                    hasInsurance = true;
                    break;
                }
            }

            if (hasInsurance)
                break;
        }
        
        // 全員終了
        SceneManager::GetInstance()
            .SetPurchasedItemsPerPlayer(
                purchasedItemsPerPlayer_);

        SceneManager::GetInstance()
            .SetPlayerMoney(
                playerMoney_);

        // ここで GameScene が参照するフラットな購入リストもセットする
        {
            std::vector<int> flat;
            flat.reserve(128);
            for (const auto& perPlayer : purchasedItemsPerPlayer_)
            {
                for (int id : perPlayer)
                {
                    flat.push_back(id);
                }
            }
            SceneManager::GetInstance().SetPurchasedItemTypes(flat);
        }

        if (bs_confirmSE != -1)
        {
            PlaySoundMem(
                bs_confirmSE,
                DX_PLAYTYPE_BACK);
        }

        scene.ChangeScene(
            SceneManager::SCENE_ID::LOADING);
    }

    if (buyMessageFrame_ > 0)
    {
        buyMessageFrame_--;
    }
}

void BuySelect::Draw(void)
{
    // 画面サイズ取得（配置位置計算で使用）
    int screenW = 0, screenH = 0;
    GetDrawScreenSize(&screenW, &screenH);

    if (isTurnChange_)
    {
        // 背景はそのまま表示
        if (bgHandle_ != -1)
        {
            DrawExtendGraph(0, 0, screenW, screenH, bgHandle_, FALSE);
        }

        // 中央に半透明ウィンドウ
        const int boxW = 700;
        const int boxH = 260;

        const int left = screenW / 2 - boxW / 2;
        const int right = screenW / 2 + boxW / 2;
        const int top = screenH / 2 - boxH / 2;
        const int bottom = screenH / 2 + boxH / 2;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 170);
        DrawBox(left, top, right, bottom, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // 枠
        DrawBox(left, top, right, bottom, GetColor(0, 255, 255), FALSE);

        // 四隅を光らせる
        DrawLine(left, top, left + 20, top, GetColor(0, 255, 255), 3);
        DrawLine(left, top, left, top + 20, GetColor(0, 255, 255), 3);

        DrawLine(right, top, right - 20, top, GetColor(0, 255, 255), 3);
        DrawLine(right, top, right, top + 20, GetColor(0, 255, 255), 3);

        DrawLine(left, bottom, left + 20, bottom, GetColor(0, 255, 255), 3);
        DrawLine(left, bottom, left, bottom - 20, GetColor(0, 255, 255), 3);

        DrawLine(right, bottom, right - 20, bottom, GetColor(0, 255, 255), 3);
        DrawLine(right, bottom, right, bottom - 20, GetColor(0, 255, 255), 3);

        unsigned int playerColor[] =
        {
            GetColor(255,80,80),
            GetColor(80,160,255),
            GetColor(80,255,80),
            GetColor(255,220,80)
        };

        DrawFormatStringToHandle(
            screenW / 2 - 180,
            screenH / 2 - 80,
            GetColor(255, 255, 255),
            fontLarge_,
            "PLAYER % d 購入完了！",
            previousPlayer_ + 1);

        bool blink = ((GetNowCount() / 300) % 2) == 0;

        if (blink)
        {
            DrawFormatStringToHandle(
                screenW / 2 - 170,
                top + 105,
                playerColor[currentPlayer_],
                fontLarge_,
                "PLAYER %d の番です",
                currentPlayer_ + 1);
        }

        DrawFormatStringToHandle(
            screenW / 2 - 120,
            screenH / 2 + 70,
            GetColor(180, 255, 255),
            fontMid_,
            "SPACEで開始");

        return;
    }
    //
    if (buyMessageFrame_ > 0)
    {
        DrawString(
            700,
            600,
            buyMessage_.c_str(),
            GetColor(255, 100, 100)
        );
    }

    if (currentPlayer_ >= playerItems_.size())
    {
        return;
    }

    

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

    unsigned int playerColor[] =
    {
        GetColor(255, 80, 80),   // Player1 赤
        GetColor(80, 160, 255),  // Player2 青
        GetColor(80, 255, 80),   // Player3 緑
        GetColor(255, 220, 80)   // Player4 黄
    };

    DrawFormatStringToHandle(
        screenW / 2 - 180,
        75,
        playerColor[currentPlayer_],
        fontLarge_,
        "PLAYER %d の購入ターン",
        currentPlayer_ + 1);

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

    const auto& items =
        playerItems_[currentPlayer_];

    for (int i = 0; i < (int)items.size(); ++i)
    {
		// 予算内で購入可能かどうかを判定
        bool canBuy =
            total + items[i].price <=
            playerMoney_[currentPlayer_];

        // 1. デフォルトの文字色を「やわらかい水色」に
        unsigned int textColor = GetColor(160, 220, 240);
        if (items[i].quantity == 0 && !canBuy)
        {
            textColor = GetColor(60, 90, 100); // 予算不足は「暗い青グレー」でグレーアウト
        }

        bool blink = ((GetNowCount() / 300) % 2) == 0;
        int rowX = listLeft + 20;
        int rowY = listTop + 14 + (i * 38);

        // 2. 選択中（カート入り）の行ハイライト（半透明のサイバー水色）
        if (items[i].quantity > 0)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 40); // 薄く透けさせる（40/255）
            DrawBox(listLeft + 8, rowY - 4, listRight - 8, rowY + 24, GetColor(0, 200, 255), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            // 数量を右寄せで表示
            char qbuf[16];
            snprintf(qbuf, sizeof(qbuf), "x%d", items[i].quantity);
            DrawFormatStringToHandle(listRight - 56, rowY, GetColor(0, 255, 200), fontMid_, "%s", qbuf);
        }

        // 3. カーソル位置（フォーカス）の処理
        if (i == cursorIdx_)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 70);

            DrawBox(
                listLeft + 8,
                rowY - 2,
                listRight - 8,
                rowY + 24,
                GetColor(0, 255, 255),
                TRUE);

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            if (blink)
            {
                DrawTriangle(
                    rowX - 8, rowY + 10,   // 右（先端）
                    rowX - 22, rowY + 2,    // 左上
                    rowX - 22, rowY + 18,   // 左下
                    GetColor(0, 255, 255),
                    TRUE);
            }

            textColor = GetColor(255, 255, 255);
        }

        // 4. アイテム名と価格の描画
        DrawFormatStringToHandle(rowX, rowY, textColor, fontMid_, "%s : %d G", items[i].name.c_str(), items[i].price);
    }

    // 経済情報（右上のステータスボックス、フォントは中）
    const int statusBoxW = 340;
    const int statusBoxH = 120;
    const int statusBoxX = screenW - statusBoxW - 36;
    const int statusBoxY = 26;

    {
        int total = CalculateTotalPrice();
        int remaining =playerMoney_[currentPlayer_] - total;
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
            DrawFormatStringToHandle(statusBoxX + 14, statusBoxY + 12, goldColor, fontMid_, "所持金: %d G",playerMoney_[currentPlayer_]);
            DrawFormatStringToHandle(statusBoxX + 14, statusBoxY + 40, cartColor, fontMid_, "カート合計: %d G", total);
            DrawFormatStringToHandle(statusBoxX + 14, statusBoxY + 68, remainColor, fontMid_, "購入後の残り: %d G", remaining);
        }
        else
        {
            DrawFormatString(statusBoxX + 12, statusBoxY + 8, goldColor, "所持金: %d G", playerMoney_[currentPlayer_]);
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
        if (previewIdx >= (int)items.size())
        {
            previewIdx = (int)items.size() - 1;
        }
        const Item& currentItem = items[previewIdx];

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
            desc = "死亡してもスコアを保持する。得点は少し低い。";
            break;
        case ITEM_TYPE::HELMET:
            desc = "被ダメージを軽減する装備。生存率アップ。";
            break;
        case ITEM_TYPE::FRAG_GRENADE:
            desc = "敵を吹き飛ばす爆弾。近距離で高ダメージ。";
            break;
        case ITEM_TYPE::SPIKE_TRAP:
            desc = "床に設置して敵を足止めする罠。";
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

    if (buyMessageFrame_ > 0)
    {
        const int boxX = 780;
        const int boxY = 550;
        const int boxW = 470;
        const int boxH = 150;

        // キャラクター画像（右上）
        if (messageFaceHandle_ != -1)
        {
            DrawExtendGraph(
                1060,   // ← 左へ60
                390,    // ← 上へ30
                1260,   // ← サイズ少し拡大
                boxY + 130,
                messageFaceHandle_,
                TRUE);
        }

        // 背景
        SetDrawBlendMode(
            DX_BLENDMODE_ALPHA,
            220);

        // 名前プレート
        const int nameX = boxX + 20;
        const int nameY = boxY - 42;
        const int nameW = 180;
        const int nameH = 36;

        // 背景
        DrawBox(
            nameX,
            nameY,
            nameX + nameW,
            nameY + nameH,
            GetColor(20, 20, 30),
            TRUE);

        // 枠
        DrawBox(
            nameX,
            nameY,
            nameX + nameW,
            nameY + nameH,
            GetColor(0, 220, 255),
            FALSE);

        // 名前
        DrawString(
            nameX + 12,
            nameY + 7,
            "デイブ(Dave)",
            GetColor(255, 220, 80));


        DrawBox(
            boxX,
            boxY,
            boxX + boxW,
            boxY + boxH,
            GetColor(20, 20, 30),
            TRUE);


        SetDrawBlendMode(
            DX_BLENDMODE_NOBLEND,
            0);


        // 枠
        DrawBox(
            boxX,
            boxY,
            boxX + boxW,
            boxY + boxH,
            GetColor(0, 220, 255),
            FALSE);
       
        // メッセージ
        DrawStringToHandle(
            boxX + 30,
            boxY + 50,
            buyMessage_.c_str(),
            GetColor(255, 100, 100),
            messageFont_);
    }
}

void BuySelect::Release(void)
{
    playerItems_.clear();
    purchasedItemsPerPlayer_.clear();
    playerMoney_.clear();

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

    if (messageFont_ != -1)
    {
        DeleteFontToHandle(messageFont_);
        messageFont_ = -1;
    }
}

bool BuySelect::IsCameraItem(ITEM_TYPE type) const
{
    switch (type)
    {
    case ITEM_TYPE::NORMAL_CAMERA:
    case ITEM_TYPE::ZOOM_CAMERA:
    case ITEM_TYPE::INSURANCE_CAMERA:
        return true;
    }

    return false;
}

bool BuySelect::HasCamera()
{
    const auto& items =
        SceneManager::GetInstance()
        .GetPurchasedItemTypes();


    for (auto item : items)
    {
        if (IsCameraItem(static_cast<ITEM_TYPE>(item)))
        {
            return true;
        }
    }

    return false;
}

void BuySelect::BuyItem(ITEM_TYPE type)
{
    // カメラの場合
    if (IsCameraItem(type))
    {
        if (HasCamera())
        {
        
            return;
        }
    }


    // 通常購入処理
    SceneManager::GetInstance()
        .AddPurchasedItemType(
            static_cast<int>(type));

}
int BuySelect::CalculateTotalPrice() const
{
    int total = 0;

    const auto& items =
        playerItems_[currentPlayer_];

    for (const auto& item : items)
    {
        total += item.price * item.quantity;
    }

    return total;
}

void BuySelect::ToggleItemSelection(int idx)
{
    // 互換用: Z=増加、X=減少 を使うためここは簡易 no-op
    (void)idx;
}