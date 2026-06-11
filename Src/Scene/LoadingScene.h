#pragma once
#include "SceneBase.h"
#include <DxLib.h>

class LoadingScene : public SceneBase
{
public:
	LoadingScene();
	~LoadingScene(void) override;

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

private:
	// ロードにかける合計時間（秒）
	static constexpr float LOAD_DURATION = 1.5f;

	// 進捗（0.0f - 1.0f）
	float progress_ = 0.0f;

	// ロード開始フラグ（ChangeScene 呼び出しを一度だけ行うため）
	bool isRequested_ = false;

	// 描画用フォント等（将来的に使用）
	int dummyHandle_ = -1;
};