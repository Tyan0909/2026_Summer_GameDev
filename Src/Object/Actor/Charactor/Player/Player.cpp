#include "Player.h"
#include "../../../../Manager/Camera.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Object/Collider/ColliderModel.h"
#include "../../../../Object/Collider/ColliderCapsule.h"
#include "../../../../Object/Collider/ColliderLine.h"
#include "../../../../Utility/AsoUtility.h"

Player::Player(void)
	:
	ActorBase(),
	gravityVelocity_(0.0f),
	isInputEnabled_(true),
	cameraAngles_(VGet(0.0f, 0.0f, 0.0f))
{
}

Player::~Player(void)
{
}

void Player::Update(void)
{
	UpdateCameraInput();
	UpdateMoveInput();

	ResolveWallCollision();
	ApplyGravity();
	ResolveWallCollision();
	transform_.Update();
}

void Player::UpdateMoveInput(void)
{
	VECTOR inputDir = AsoUtility::VECTOR_ZERO;

	if (isInputEnabled_ && CheckHitKey(KEY_INPUT_W))
	{
		inputDir.z += MOVE_SPEED;
	}
	if (isInputEnabled_ && CheckHitKey(KEY_INPUT_S))
	{
		inputDir.z -= MOVE_SPEED;
	}
	if (isInputEnabled_ && CheckHitKey(KEY_INPUT_A))
	{
		inputDir.x -= MOVE_SPEED;
	}
	if (isInputEnabled_ && CheckHitKey(KEY_INPUT_D))
	{
		inputDir.x += MOVE_SPEED;
	}

	if (!AsoUtility::EqualsVZero(inputDir))
	{
		const float moveSpeed = 1.0f;
		VECTOR moveDir = AsoUtility::VNormalize(inputDir);
		moveDir = VTransform(moveDir, MGetRotY(cameraAngles_.y));
		moveDir.y = 0.0f;

		if (!AsoUtility::EqualsVZero(moveDir))
		{
			const Quaternion targetRot = Quaternion::LookRotation(moveDir);

			float turnT = SceneManager::GetInstance().GetDeltaTime() * TURN_SPEED;
			if (turnT < 0.0f)
			{
				turnT = 0.0f;
			}
			if (turnT > 1.0f)
			{
				turnT = 1.0f;
			}

			transform_.quaRot = Quaternion::Slerp(transform_.quaRot, targetRot, turnT);
		}

		transform_.pos = VAdd(transform_.pos, VScale(moveDir, moveSpeed));
	}
}

void Player::UpdateCameraInput(void)
{
	if (!isInputEnabled_)
	{
		return;
	}

	if (CheckHitKey(KEY_INPUT_UP))
	{
		cameraAngles_.x -= CAMERA_ROT_SPEED;
	}
	if (CheckHitKey(KEY_INPUT_DOWN))
	{
		cameraAngles_.x += CAMERA_ROT_SPEED;
	}
	if (CheckHitKey(KEY_INPUT_LEFT))
	{
		cameraAngles_.y -= CAMERA_ROT_SPEED;
	}
	if (CheckHitKey(KEY_INPUT_RIGHT))
	{
		cameraAngles_.y += CAMERA_ROT_SPEED;
	}

	if (cameraAngles_.x < CAMERA_PITCH_MIN)
	{
		cameraAngles_.x = CAMERA_PITCH_MIN;
	}
	if (cameraAngles_.x > CAMERA_PITCH_MAX)
	{
		cameraAngles_.x = CAMERA_PITCH_MAX;
	}
}

const VECTOR& Player::GetCameraAngles(void) const
{
	return cameraAngles_;
}

void Player::SetCameraAngles(const VECTOR& angles)
{
	cameraAngles_ = angles;
}

VECTOR Player::GetCameraWorldPos(void) const
{
	VECTOR cameraOffset = TPS_CAMERA_LOCAL_POS;
	cameraOffset = VTransform(cameraOffset, MGetRotY(cameraAngles_.y));
	return VAdd(transform_.pos, cameraOffset);

	// àÍêlèÃéãì_Ç…ñﬂÇ∑èÍçá
	// VECTOR cameraOffset = FPS_CAMERA_LOCAL_POS;
	// cameraOffset = VTransform(cameraOffset, MGetRotY(cameraAngles_.y));
	// return VAdd(transform_.pos, cameraOffset);
}

VECTOR Player::GetCameraForward(void) const
{
	const float pitch = cameraAngles_.x;
	const float yaw = cameraAngles_.y;

	VECTOR forward = VGet(
		sinf(yaw) * cosf(pitch),
		-sinf(pitch),
		cosf(yaw) * cosf(pitch));

	const float length = VSize(forward);
	if (length <= 0.0001f)
	{
		return VGet(0.0f, 0.0f, 1.0f);
	}

	return VScale(forward, 1.0f / length);
}

void Player::SetPos(const VECTOR& pos)
{
	transform_.pos = pos;
	transform_.Update();
}

void Player::SetInputEnabled(bool isEnabled)
{
	isInputEnabled_ = isEnabled;
}

void Player::InitLoad(void)
{
	transform_.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::PLAYER));

	if (transform_.modelId == -1)
	{
		return;
	}
}

void Player::InitTransform(void)
{
	transform_.scl = { 0.5f,0.5f,0.5f };
	transform_.quaRot = Quaternion::Identity();

	transform_.quaRotLocal = Quaternion::AngleAxis(DX_PI_F, VGet(0.0f, 1.0f, 0.0f));

	transform_.pos = INIT_POS;
	transform_.Update();
}

void Player::InitCollider(void)
{
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::LINE), colLine);

	ColliderCapsule* colCapsule = new ColliderCapsule(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_CAPSULE_TOP_LOCAL_POS, COL_CAPSULE_DOWN_LOCAL_POS,
		COL_CAPSULE_RADIUS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);
}

void Player::InitAnimation(void)
{
}

void Player::InitPost(void)
{
}

void Player::ApplyGravity(void)
{
	VECTOR hitPos = AsoUtility::VECTOR_ZERO;
	if (CheckGround(hitPos) && gravityVelocity_ <= 0.0f)
	{
		transform_.pos.y = hitPos.y + GROUND_OFFSET;
		gravityVelocity_ = 0.0f;
		return;
	}

	gravityVelocity_ -= GRAVITY;
	if (gravityVelocity_ < GRAVITY_TERMINAL)
	{
		gravityVelocity_ = GRAVITY_TERMINAL;
	}

	transform_.pos.y += gravityVelocity_;

	if (CheckGround(hitPos) && gravityVelocity_ <= 0.0f)
	{
		transform_.pos.y = hitPos.y + GROUND_OFFSET;
		gravityVelocity_ = 0.0f;
	}
}

bool Player::CheckGround(VECTOR& hitPos) const
{
	const VECTOR start = VAdd(transform_.pos, VGet(0.0f, 10.0f, 0.0f));
	const VECTOR end = VAdd(transform_.pos, VGet(0.0f, -GROUND_CHECK_DISTANCE, 0.0f));

	for (const auto& hitCollider : hitColliders_)
	{
		if (hitCollider == nullptr ||
			hitCollider->GetShape() != ColliderBase::SHAPE::MODEL)
		{
			continue;
		}

		const auto* modelCollider = static_cast<const ColliderModel*>(hitCollider);
		auto hit = modelCollider->GetNearestHitPolyLine(start, end, true);
		if (hit.HitFlag)
		{
			hitPos = hit.HitPosition;
			hitPos.y += 5.0f;
			return true;
		}
	}

	return false;
}

void Player::ResolveWallCollision(void)
{
	const auto* capsule = static_cast<const ColliderCapsule*>(
		GetOwnCollider(static_cast<int>(COLLIDER_TYPE::CAPSULE)));

	if (capsule == nullptr)
	{
		return;
	}

	for (const auto& hitCollider : hitColliders_)
	{
		if (hitCollider == nullptr ||
			hitCollider->GetShape() != ColliderBase::SHAPE::MODEL)
		{
			continue;
		}

		const auto* modelCollider = static_cast<const ColliderModel*>(hitCollider);

		capsule->PushBackAlongNormal(
			modelCollider,
			transform_,
			8,
			WALL_PUSH_BACK,
			true,
			false,
			WALL_NORMAL_Y_MAX);
	}
}

