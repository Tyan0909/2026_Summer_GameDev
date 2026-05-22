#include "BuySelect.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"

BuySelect::BuySelect(void) {}
BuySelect::~BuySelect(void) {}

void BuySelect::Init(void)
{
    // パスの誤りを修正（"Data/Image/BuySelect/..." に統一）
    itemImg_[0] =
        LoadGraph("Data/Image/BuySelect/NormalCamera.png");

    itemImg_[1] =
        LoadGraph("Data/Image/BuySelect/ZoomCamera.png");

    itemImg_[2] =
        LoadGraph("Data/Image/BuySelect/InsuranceCamera.png");

    itemImg_[3] =
        LoadGraph("Data/Image/BuySelect/Helmet.png");

    itemImg_[4] =
        LoadGraph("Data/Image/BuySelect/FragGrenade.png");

    items_.clear();
    items_.push_back({ "ノーマルカメラ", 0,false,ITEM_TYPE::NORMAL_CAMERA });
    items_.push_back({ "ズームカメラ", 800, false,ITEM_TYPE::ZOOM_CAMERA });
    items_.push_back({ "保険カメラ", 1200, false,ITEM_TYPE::INSURANCE_CAMERA });
    items_.push_back({ "ヘルメット", 500, false,ITEM_TYPE::HELMET });
    items_.push_back({ "フラググレネード", 300, false,ITEM_TYPE::FRAG_GRENADE });

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
        cursorIdx_ = (cursorIdx_ - 1 + (int)items_.size()) % (int)items_.size();
    if (ins.IsTrgDown(KEY_INPUT_DOWN))
        cursorIdx_ = (cursorIdx_ + 1) % (int)items_.size();

    // Zキーで選択トグル
    if (ins.IsTrgDown(KEY_INPUT_Z))
        ToggleItemSelection(cursorIdx_);

    // SPACEキーで購入確定（清算）
    if (ins.IsTrgDown(KEY_INPUT_SPACE))
    {
        int total = CalculateTotalPrice();

        // 購入後の所持金（最低保証1500を含む）
        int remaining = currentAmount_ - total;

        // carryMoney は「最低保証を除いた持ち越し可変分」に統一する
        // 例：購入後 remaining=2700 のとき carry=2700-1500=1200
        int carry = remaining - minAmount_;
        if (carry < 0) carry = 0;

        // SceneManager に保存（これで次の Result / BuySelect に引き継がれる）
        scene.SetCarryMoney(carry);

        scene.ChangeScene(SceneManager::SCENE_ID::LOADING);
    }
}

void BuySelect::Draw(void)
{
    DrawString(200, 50, "アイテム選択・購入", GetColor(255, 255, 255));

    for (int i = 0; i < (int)items_.size(); ++i)
    {
        int total = CalculateTotalPrice();

        bool canBuy =
            total + items_[i].price <= currentAmount_;

        unsigned int color = GetColor(255, 255, 255);

        // カーソル位置の点滅用フラグ（300msごとに切り替え）
        bool blink = ((GetNowCount() / 300) % 2) == 0;

        if (i == cursorIdx_)
        {
            // カーソル位置は点滅（黄/白）
            color = blink ? GetColor(255, 255, 0) : GetColor(255, 255, 255);
        }

        if (!items_[i].isSelected && !canBuy)
        {
            // 購入不可は赤（カーソル位置の点滅を優先したければここを移動してください）
            color = GetColor(255, 80, 80);
        }

        // 選択マークはカーソル時に点滅（非表示にする）
        std::string mark = items_[i].isSelected ? "[■]" : "[ ]";
        if (i == cursorIdx_ && !blink)
        {
            mark = "   "; // 点滅時はマークを非表示にする
        }

        DrawFormatString(
            200,
            120 + (i * 40),
            color,
            "%s %s : %d G",
            mark.c_str(),
            items_[i].name.c_str(),
            items_[i].price
        );
    }

    int total = CalculateTotalPrice();
    DrawFormatString(200, 350, GetColor(0, 255, 255),
        "合計: %d G (残り: %d G)", total, currentAmount_ - total);

    const Item& currentItem = items_[cursorIdx_];

    // 半透明ウィンドウ
    SetDrawBlendMode(
        DX_BLENDMODE_ALPHA,
        180
    );

    DrawBox(
        700,
        120,
        1150,
        300,
        GetColor(30, 30, 30),
        TRUE
    );

    SetDrawBlendMode(
        DX_BLENDMODE_NOBLEND,
        0
    );

    DrawBox(
        700,
        120,
        1150,
        300,
        GetColor(255, 255, 255),
        FALSE
    );

    // 画像の「ふわふわ」(sinによる揺れ) を止める -> 固定Yにする
    int imgY = 180;

    DrawGraph(
        730,
        imgY,
        itemImg_[(int)currentItem.type],
        TRUE
    );

    DrawFormatString(
        730,
        140,
        GetColor(255, 255, 0),
        "%s",
        currentItem.name.c_str()
    );

    switch (currentItem.type)
    {
    case ITEM_TYPE::NORMAL_CAMERA:

        DrawString(
            730,
            190,
            "標準的なカメラ",
            GetColor(255, 255, 255)
        );

        break;

    case ITEM_TYPE::ZOOM_CAMERA:

        DrawString(
            730,
            190,
            "遠距離ズーム撮影が可能",
            GetColor(255, 255, 255)
        );

        break;

    case ITEM_TYPE::INSURANCE_CAMERA:

        DrawString(
            730,
            190,
            "死亡してもスコアを保持する",
            GetColor(255, 255, 255)
        );

        break;

    case ITEM_TYPE::HELMET:

        DrawString(
            730,
            190,
            "被ダメージを軽減",
            GetColor(255, 255, 255)
        );

        break;

    case ITEM_TYPE::FRAG_GRENADE:

        DrawString(
            730,
            190,
            "敵を吹き飛ばす爆弾",
            GetColor(255, 255, 255)
        );

        break;
    }

    // 参考表示：次に持ち越される「可変分（最低保証を除いた値）」も見えるようにするなら：
    // int carryPreview = (currentAmount_ - total) - minAmount_;
    // if (carryPreview < 0) carryPreview = 0;
    // DrawFormatString(200, 380, GetColor(255, 255, 255), "持ち越し(可変分): %d G", carryPreview);
}

void BuySelect::Release(void)
{
    items_.clear();
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