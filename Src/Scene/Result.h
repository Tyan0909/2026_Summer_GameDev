#pragma once
#include "SceneBase.h"
class Result : public SceneBase
{
public:
	// コンストラクタ
	Result(void);

	// デストラクタ
	~Result(void) override;
	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

	// ゲームシーンで獲得した残額を取得する関数
	// Result -> SceneManager に「今回の獲得分」を渡す
	// 例：SceneManager::GetInstance().SetCarryMoney(resultAmount);

private:

	// ここにメンバ変数を追加していく
	int playerWinID_;
	int playerScores_[4];
};

