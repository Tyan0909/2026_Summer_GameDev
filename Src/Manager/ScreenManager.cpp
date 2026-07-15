#include "ScreenManager.h"
#include <utility>

ScreenManager::ScreenManager()
	: leftScreenHandle_(-1),
	rightScreenHandle_(-1),
	bottomLeftScreenHandle_(-1),
	bottomRightScreenHandle_(-1),
	sceneScreenHandle_(-1),
	screenshotScreenHandle_(-1)
{
}

ScreenManager::~ScreenManager()
{
	Release();
}

void ScreenManager::Create(int screenW, int screenH, int selectedPlayerCount, bool isSplitScreenEnabled)
{
	// 既存のハンドルを解放
	ResetHandles();

	// 分割画面の有効化とプレイヤー数に応じてスクリーンを作成
	// 2人時は左右、3人以上は4分割
	if (isSplitScreenEnabled && selectedPlayerCount == 2)
	{
		leftScreenHandle_ = MakeScreen(screenW / 2, screenH, TRUE);
		rightScreenHandle_ = MakeScreen(screenW / 2, screenH, TRUE);
		bottomLeftScreenHandle_ = -1;
		bottomRightScreenHandle_ = -1;
	}
	else if (isSplitScreenEnabled && selectedPlayerCount >= 3)
	{
		leftScreenHandle_ = MakeScreen(screenW / 2, screenH / 2, TRUE);
		rightScreenHandle_ = MakeScreen(screenW / 2, screenH / 2, TRUE);
		bottomLeftScreenHandle_ = MakeScreen(screenW / 2, screenH / 2, TRUE);
		bottomRightScreenHandle_ = MakeScreen(screenW / 2, screenH / 2, TRUE);
	}
	else
	{
		leftScreenHandle_ = -1;
		rightScreenHandle_ = -1;
		bottomLeftScreenHandle_ = -1;
		bottomRightScreenHandle_ = -1;
	}

	// シーン全体とスクショは常に生成
	sceneScreenHandle_ = MakeScreen(screenW, screenH, TRUE);
	screenshotScreenHandle_ = MakeScreen(screenW, screenH, TRUE);
}

void ScreenManager::Release()
{
	DeleteHandle(leftScreenHandle_);
	DeleteHandle(rightScreenHandle_);
	DeleteHandle(bottomLeftScreenHandle_);
	DeleteHandle(bottomRightScreenHandle_);
	DeleteHandle(sceneScreenHandle_);
	DeleteHandle(screenshotScreenHandle_);
}

void ScreenManager::ResetHandles()
{
	DeleteHandle(leftScreenHandle_);
	DeleteHandle(rightScreenHandle_);
	DeleteHandle(bottomLeftScreenHandle_);
	DeleteHandle(bottomRightScreenHandle_);
	DeleteHandle(sceneScreenHandle_);
	DeleteHandle(screenshotScreenHandle_);
}

void ScreenManager::DeleteHandle(int& handle)
{
	if (handle != -1)
	{
		DeleteGraph(handle);
		handle = -1;
	}
}

int ScreenManager::GetPlayerViewHandle(int playerIndex) const
{
	// playerIndex によって適切な分割ハンドルを返す。
	if (leftScreenHandle_ == -1 && sceneScreenHandle_ != -1)
	{
		return sceneScreenHandle_;
	}

	// 2人分割
	if (leftScreenHandle_ != -1 && rightScreenHandle_ != -1 && bottomLeftScreenHandle_ == -1)
	{
		return (playerIndex == 0) ? leftScreenHandle_ : rightScreenHandle_;
	}

	// 4分割想定（存在しない場合は scene を返す）
	switch (playerIndex)
	{
	case 0: return (leftScreenHandle_ != -1) ? leftScreenHandle_ : sceneScreenHandle_;
	case 1: return (rightScreenHandle_ != -1) ? rightScreenHandle_ : sceneScreenHandle_;
	case 2: return (bottomLeftScreenHandle_ != -1) ? bottomLeftScreenHandle_ : sceneScreenHandle_;
	case 3: return (bottomRightScreenHandle_ != -1) ? bottomRightScreenHandle_ : sceneScreenHandle_;
	default: return sceneScreenHandle_;
	}
}

void ScreenManager::Compose(bool isFourWay, int screenWidth, int screenHeight)
{
	if (sceneScreenHandle_ == -1)
	{
		return;
	}

	SetDrawScreen(sceneScreenHandle_);
	SetDrawArea(0, 0, screenWidth, screenHeight);
	ClearDrawScreen();

	// 左・右は無条件に描く（ハンドルが -1 の場合は描画は noop）
	if (leftScreenHandle_ != -1) DrawGraph(0, 0, leftScreenHandle_, FALSE);
	if (rightScreenHandle_ != -1) DrawGraph(screenWidth / 2, 0, rightScreenHandle_, FALSE);

	if (isFourWay)
	{
		if (bottomLeftScreenHandle_ != -1) DrawGraph(0, screenHeight / 2, bottomLeftScreenHandle_, FALSE);
		if (bottomRightScreenHandle_ != -1) DrawGraph(screenWidth / 2, screenHeight / 2, bottomRightScreenHandle_, FALSE);

		// 仕切り線（水平）
		DrawBox(
			0,
			screenHeight / 2 - 1,
			screenWidth,
			screenHeight / 2 + 1,
			GetColor(255, 255, 255),
			TRUE);
	}

	// 仕切り線（垂直）
	DrawBox(
		screenWidth / 2 - 1,
		0,
		screenWidth / 2 + 1,
		screenHeight,
		GetColor(255, 255, 255),
		TRUE);
}
