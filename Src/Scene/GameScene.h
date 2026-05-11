#pragma once
#include "SceneBase.h"
#include "../Object/Actor/Stage/Stage.h" 

class Grid;
class StageManager;
class PlayerManager;
class Stage;
class Player_1;
class Player;
class Subject;

class GameScene : public SceneBase
{
public:
	GameScene();
	~GameScene(void) override;

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Draw3D(void);
	void Release(void) override;

private:
	static constexpr VECTOR FPS_CAMERA_LOCAL_POS = { 0.0f, 90.0f, 5.0f };
	static constexpr float FPS_CAMERA_ROT_SPEED = DX_PI_F / 180.0f;
	static constexpr float FPS_CAMERA_PITCH_MIN = -DX_PI_F * 0.45f;
	static constexpr float FPS_CAMERA_PITCH_MAX = DX_PI_F * 0.45f;
	static constexpr VECTOR PLAYER2_INIT_POS = { 200.0f, 1000.0f, 0.0f };

	void DrawSplitView(int screenHandle, const Player* targetPlayer, const VECTOR& cameraAngles, const Player* hidePlayer);
	void DrawSingleView(const Player* targetPlayer, const VECTOR& cameraAngles, const Player* hidePlayer);

	Stage* stage_;

	Player* player_;
	Player* player2_;
	Subject* subject_;
	int leftScreenHandle_;
	int rightScreenHandle_;
	int screenWidth_;
	int screenHeight_;
	VECTOR player1CameraAngles_;
	VECTOR player2CameraAngles_;
	bool isSplitScreenEnabled_;
};
