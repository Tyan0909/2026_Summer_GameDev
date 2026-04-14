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

private:

	// ここにメンバ変数を追加していく
	int playerWinID_;
	int playerScores_[4];
};

