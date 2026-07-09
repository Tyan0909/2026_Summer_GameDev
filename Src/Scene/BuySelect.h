#pragma once
#include "SceneBase.h"
#include <DxLib.h>
#include <vector>
#include <string>		// std::string

// アイテム種類
enum class ITEM_TYPE
{
	NORMAL_CAMERA,
	ZOOM_CAMERA,
	INSURANCE_CAMERA,
	HELMET,
	FRAG_GRENADE,
	SPIKE_TRAP,
	EXPLOSIVE_TRAP,
};


// アイテム情報（数量対応）
struct Item
{
	std::string name;		// アイテム名
	int price;				// 価格
	int quantity;			// 購入数量（0 = 未選択）
	ITEM_TYPE type;
};

class BuySelect : public SceneBase
{
public:
	// 所持等
	static const int MAX_AMOUNT = 10000;	// 最大金額
	static const int MIN_AMOUNT = 500;     // 最低保証
	static constexpr int TURN_CHANGE_TIME = 120; // 2秒(60FPS)

	BuySelect(void);
	~BuySelect(void) override;

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

	bool IsCameraItem(ITEM_TYPE type) const;	// カメラアイテムかどうか
	bool HasCamera();
	void BuyItem(ITEM_TYPE type);

private:
	int CalculateTotalPrice() const;	// カート合計
	void ToggleItemSelection(int idx);	// 既存互換（未使用でも可）

private:

	int itemImg_[7];

	// 背景
	int bgHandle_ = -1;

	// フォント
	int fontLarge_ = -1;
	int fontMid_ = -1;
	int fontSmall_ = -1;

	int previousPlayer_ = 0;

	// カーソル
	int cursorIdx_ = 0;
	std::string buyMessage_;
	int buyMessageFrame_ = 0;

	int currentPlayer_ = 0;

	bool isTurnChange_ = true;      // 手番切替画面中か
	int turnChangeFrame_ = 0;       // フレーム数

	// プレイヤーごとの購入リスト
	std::vector<std::vector<Item>> playerItems_;
	// 現在のプレイヤーの購入リストへの参照
	std::vector<std::vector<int>> purchasedItemsPerPlayer_;
	// プレイヤーごとの所持金
	std::vector<int> playerMoney_;

	// 最低保証
	int minAmount_ = MIN_AMOUNT;
};