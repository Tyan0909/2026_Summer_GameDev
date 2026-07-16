#pragma once
#include <vector>
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


	struct RankData
	{
		int playerNo;
		int score;
	};

	std::vector<RankData> ranking_;

	// ここにメンバ変数を追加していく
	int playerWinID_;
	int playerScores_[4];
	int bestPhotoHandle_ = -1;
	int bestPhotoScore_ = 0;
	int bestPhotoPlayer_ = -1;
	int titleFont_ = -1;
	int rankFont_ = -1;
	int resultFrame_ = 0;
	int photoY_;
	int displayScore_ = 0;
	int scoreDisplay_ = 0;
	bool scoreFinished_ = false;
	int shineFrame_ = 0;
	int rankingStartFrame_ = 150;

	bool isClear_;
};

