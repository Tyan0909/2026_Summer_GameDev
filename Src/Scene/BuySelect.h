#pragma once
#include "SceneBase.h"
#include <DxLib.h>

class BuySelect : public SceneBase
{
public:

	// 定数
	static const int MAX_AMOUNT = 10000;	// 最大残額
	static const int MIN_AMOUNT = 1500;     // 最小残額

	// アイテムの分類
	enum class ITEM_TYPE
	{
		
	};

	// コンストラクタ
	BuySelect(void);

	// デストラクタ
	~BuySelect(void) override;

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

	// 前回の残額を取得する関数
	// Rsult -> 前回の残額
	int GetPrevAmount(void)
	{
		return prevAmount;
	}

private:
	// ここにメンバ変数を追加していく

	// 前回の残額を保存する変数
	int prevAmount = 0;

	// 現在の残額を保存する変数
	int currentAmount = 0;
};