#pragma once
#include "SceneBase.h"
class Grid;

class TitleScene : public SceneBase
{

public:

	// コンストラクタ
	TitleScene(void);

	// デストラクタ
	~TitleScene(void) override;

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

private:
	// デバッグ用グリッド
	Grid* debugGrid_;

	int logoHandle_;      // ロゴ画像
	int pointerHandle_;   // ポインタ画像
	int shutterSE_;       // シャッター音

	float shutterScale_;  // シャッター縮小
	bool isShutter_;      // シャッター中

	int flashAlpha_;      // フラッシュ

	VECTOR pointerPos_;   // ポインタ位置
	VECTOR targetPos_;    // ロックオン先
};


