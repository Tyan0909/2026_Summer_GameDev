#include "BuySelect.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"

BuySelect::BuySelect(void) {}
BuySelect::~BuySelect(void) {}

void BuySelect::Init(void)
{
    items_.clear();
    items_.push_back({ "アイテムA", 500, false });
    items_.push_back({ "アイテムB", 100, false });
    items_.push_back({ "アイテムC", 400, false });
    items_.push_back({ "アイテムD", 200, false });

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

        scene.ChangeScene(SceneManager::SCENE_ID::GAME);
    }
}

void BuySelect::Draw(void)
{
    DrawString(200, 50, "アイテム選択・購入", GetColor(255, 255, 255));

    for (int i = 0; i < (int)items_.size(); ++i)
    {
        unsigned int color = (i == cursorIdx_) ? GetColor(255, 255, 0) : GetColor(255, 255, 255);
        const char* mark = items_[i].isSelected ? "[■] " : "[  ] ";
        DrawFormatString(200, 120 + (i * 30), color, "%s %-12s : %d G",
            mark, items_[i].name.c_str(), items_[i].price);
    }

    int total = CalculateTotalPrice();
    DrawFormatString(200, 350, GetColor(0, 255, 255),
        "合計: %d G (残り: %d G)", total, currentAmount_ - total);

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