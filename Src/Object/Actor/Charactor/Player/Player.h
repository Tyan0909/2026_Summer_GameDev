#pragma once
#include <DxLib.h>
#include <vector>
#include <string>
#include "../../ActorBase.h"
#include "../../../../Object/Collider/ColliderModel.h"

class ColliderBase;
class ResourceManager;
class AnimationController;

class Player : public ActorBase
{
public:

	// 定数
	static constexpr float GRAVITY = 0.5f;
	static constexpr float MOVE_SPEED = 150.f;

	static constexpr VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 80.0f, 0.0f };
	static constexpr VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, -10.0f, 0.0f };

	static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS = { 0.0f, 110.0f, 0.0f };
	static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS = { 0.0f, 30.0f, 0.0f };
	static constexpr float COL_CAPSULE_RADIUS = 20.0f;

	// 三人称視点
	static constexpr VECTOR TPS_CAMERA_LOCAL_POS = { 0.0f, 120.0f, -180.0f };

	// 一人称視点
	// static constexpr VECTOR FPS_CAMERA_LOCAL_POS = { 0.0f, 90.0f, 5.0f };

	enum class COLLIDER_TYPE
	{
		MODEL,
		LINE,
		CAPSULE,
		MAX,
	};

	enum class ANIM_TYPE
	{
		IDLE,
		RUN,
		JUMP,
		CROUCHED,
		WALK,
		MAX,

	};

	enum class STATE
	{
		IDLE,
		WALK,
		RUN,
		JUMP,
		CROUCHED,
	};

	Player(void);
	~Player(void);

	void Init(void) ;
	void Update(void) override;
	void SetPos(const VECTOR& pos);
	void SetInputEnabled(bool isEnabled);

	const VECTOR& GetCameraAngles(void) const;
	void SetCameraAngles(const VECTOR& angles);
	VECTOR GetCameraWorldPos(void) const;
	VECTOR GetCameraForward(void) const;



protected:
	void InitLoad(void) override;
	void InitTransform(void) override;
	void InitCollider(void) override;
	void InitAnimation(void) override;
	void InitPost(void) override;

private:
	static constexpr float GRAVITY_TERMINAL = -20.0f;
	static constexpr float GROUND_CHECK_DISTANCE = 500.0f;
	static constexpr float GROUND_OFFSET = 1.0f;
	static constexpr float WALL_CHECK_HEIGHT = 30.0f;
	static constexpr float WALL_PUSH_BACK = 2.0f;
	static constexpr float WALL_NORMAL_Y_MAX = 0.4f;

	static constexpr float CAMERA_ROT_SPEED = DX_PI_F / 180.0f;
	static constexpr float CAMERA_PITCH_MIN = -DX_PI_F * 0.45f;
	static constexpr float CAMERA_PITCH_MAX = DX_PI_F * 0.45f;
	static constexpr float TURN_SPEED = 10.0f;

	static constexpr VECTOR INIT_POS = { 300.0f, 100.0f, 100.0f };

	float gravityVelocity_;
	bool isInputEnabled_;
	VECTOR cameraAngles_;

	AnimationController* animController_;

	void UpdateMoveInput(void);
	void UpdateCameraInput(void);
	void ApplyGravity(void);
	bool CheckGround(VECTOR& hitPos) const;
	void ResolveWallCollision(void);

	STATE state_;
};

