#include "LoadingScene.h"
#include "../Manager/SceneManager.h"
#include <DxLib.h>

LoadingScene::LoadingScene()
	: SceneBase()
{
}

LoadingScene::~LoadingScene(void)
{
}

void LoadingScene::Init(void)
{
	progress_ = 0.0f;
	isRequested_ = false;
	// 必要ならフォントや画像をここで読み込み（現状ダミー）
	dummyHandle_ = -1;
}

void LoadingScene::Update(void)
{
	// SceneManager 側で deltaTime を更新済みなので取得して進捗を進める
	float dt = SceneManager::GetInstance().GetDeltaTime();

	progress_ += dt / LOAD_DURATION;
	if (progress_ > 1.0f) progress_ = 1.0f;

	// 進捗完了したら一度だけゲームシーンへ遷移を要求
	if (progress_ >= 1.0f && !isRequested_)
	{
		isRequested_ = true;
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
	}
}

void LoadingScene::Draw(void)
{
	// 背景をクリア（SceneManager::Draw で既にバックバッファ設定済み）
	ClearDrawScreen();

	// 黒背景
	DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);

	// Loading テキスト
	DrawString(540, 300, "Loading...", GetColor(255, 255, 255));

	// 進捗バー枠
	const int barX = 240;
	const int barY = 360;
	const int barW = 800;
	const int barH = 36;

	DrawBox(barX - 2, barY - 2, barX + barW + 2, barY + barH + 2, GetColor(255, 255, 255), FALSE);

	// 進捗バー中身
	int filledW = static_cast<int>(barW * progress_);
	DrawBox(barX, barY, barX + filledW, barY + barH, GetColor(100, 200, 255), TRUE);

	// ％表示
	char buf[64];
	sprintf_s(buf, "%d %%", static_cast<int>(progress_ * 100.0f));
	DrawString(barX + (barW / 2) - 20, barY + 6, buf, GetColor(0, 0, 0));
}

void LoadingScene::Release(void)
{
	// 読み込んだリソースがあれば解放（今は無し）
}