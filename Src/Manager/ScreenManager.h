#pragma once
#include <DxLib.h>

class ScreenManager
{
public:

	ScreenManager();
	~ScreenManager();

	// 生成・解放
	void Create(int screenWidth, int screenHeight, int selectedPlayerCount, bool isSplitScreenEnabled);
	void Release();

	// プレイヤービューに対応するハンドルを取得
	int GetPlayerViewHandle(int playerIndex) const;
	//int GetSceneHandle() const { return sceneScreenHandle_; }
	int GetScreenshotHandle() const { return screenshotScreenHandle_; }

	// 合成処理
	void Compose(bool isFourWay, int screenW, int screenH);

	// 補助関数
	void Clear() { Release(); }

private:

	void ResetHandles();
	void DeleteHandle(int& handle);

	int leftScreenHandle_;			// 2人時は左、4人時は左上
	int rightScreenHandle_;			// 2人時は右、4人時は右上
	int bottomLeftScreenHandle_;	// 4人時は左下
	int bottomRightScreenHandle_;   // 4人時は右下
	int sceneScreenHandle_;			// 全体描画用
	int screenshotScreenHandle_;	// 撮影用
};

