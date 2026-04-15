#pragma once
#include "SceneBase.h"
#include <DxLib.h>
#include <vector>
#include <string>		// std::string

// アイテムの情報を管理する構造体
struct Item
{
	std::string name;		// アイテム名
	int price; 				// アイテム価格
	bool isSelected;		// 選択中か
};

class BuySelect : public SceneBase
{
public:
	// 定数
	static const int MAX_AMOUNT = 10000;	// 最大所持金
	static const int MIN_AMOUNT = 500;     // 最低保証

	BuySelect(void);
	~BuySelect(void) override;

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

private:
	int CalculateTotalPrice() const;	// 合計金額計算
	void ToggleItemSelection(int idx);	// 選択切替

private:
	// 現在の所持金（= 最低保証 + carryMoney）
	int currentAmount_ = 0;

	// カーソル位置
	int cursorIdx_ = 0;

	// 最低保証所持金
	int minAmount_ = MIN_AMOUNT;

	// 購入アイテムリスト
	std::vector<Item> items_;
};