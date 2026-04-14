#pragma once
#include "SceneBase.h"
#include "../Object/Stage/Stage.h" 

class Grid;
class StageManager;
class PlayerManager;
class Stage;
class Player_1;

class GameScene : public SceneBase
{
public:

	// コンストラクタ
	GameScene();
	// デストラクタ
	~GameScene(void)override;

	// 初期化
	void Init(void)override;
	// 更新
	void Update(void)override;
	// 描画
	void Draw(void)override;
	// 3D描画
	void Draw3D(void);
	// リソースの破棄
	void Release(void)override;

private:

	// ここにメンバ変数を追加していく
};
