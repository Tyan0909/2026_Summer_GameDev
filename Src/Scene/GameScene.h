#pragma once
#include "SceneBase.h"
#include "../Object/Actor/Stage/Stage.h" 

class Grid;
class StageManager;
class PlayerManager;
class Stage;
class Player_1;
class Player;

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
	static constexpr VECTOR FPS_CAMERA_LOCAL_POS = { 0.0f, 90.0f, 5.0f };
	static constexpr float FPS_CAMERA_ROT_SPEED = DX_PI_F / 180.0f;
	static constexpr float FPS_CAMERA_PITCH_MIN = -DX_PI_F * 0.45f;
	static constexpr float FPS_CAMERA_PITCH_MAX = DX_PI_F * 0.45f;
	static constexpr VECTOR PLAYER2_INIT_POS = { 200.0f, 1000.0f, 0.0f };

	void DrawSplitView(int screenHandle, const Player* targetPlayer, const VECTOR& cameraAngles, const Player* hidePlayer);

	// ここにメンバ変数を追加していく
	Stage* stage_;

	Player* player_;
	Player* player2_;
	int leftScreenHandle_;
	int rightScreenHandle_;
	int screenWidth_;
	int screenHeight_;
	VECTOR player1CameraAngles_;
	VECTOR player2CameraAngles_;

	
};
