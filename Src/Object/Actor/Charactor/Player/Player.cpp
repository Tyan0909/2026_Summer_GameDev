#include "Player.h"
#include <cmath>
#include "../../../../Manager/Camera.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Object/Common/AnimationController.h"
#include "../../../../Object/Collider/ColliderModel.h"
#include "../../../../Object/Collider/ColliderCapsule.h"
#include "../../../../Object/Collider/ColliderLine.h"
#include "../../../../Utility/AsoUtility.h"
#include "../../../../Application.h"

const Player::INPUT_CONFIG Player::KEYBOARD_INPUT_CONFIG =
{
	INPUT_DEVICE::KEYBOARD,
	KEY_INPUT_W,
	KEY_INPUT_S,
	KEY_INPUT_A,
	KEY_INPUT_D,
	KEY_INPUT_UP,
	KEY_INPUT_DOWN,
	KEY_INPUT_LEFT,
	KEY_INPUT_RIGHT,
	KEY_INPUT_C,
	KEY_INPUT_LSHIFT,
	InputManager::JOYPAD_NO::PAD1,
	InputManager::JOYPAD_BTN::R_BUMPER,
	InputManager::JOYPAD_BTN::DOWN
};

const Player::INPUT_CONFIG Player::PAD1_INPUT_CONFIG =
{
	INPUT_DEVICE::PAD,
	KEY_INPUT_W,
	KEY_INPUT_S,
	KEY_INPUT_A,
	KEY_INPUT_D,
	KEY_INPUT_UP,
	KEY_INPUT_DOWN,
	KEY_INPUT_LEFT,
	KEY_INPUT_RIGHT,
	KEY_INPUT_C,
	KEY_INPUT_LSHIFT,
	InputManager::JOYPAD_NO::PAD1,
	InputManager::JOYPAD_BTN::R_BUMPER,
	InputManager::JOYPAD_BTN::DOWN
};

const Player::INPUT_CONFIG Player::KEYBOARD_AND_PAD1_INPUT_CONFIG =
{
	INPUT_DEVICE::BOTH,
	KEY_INPUT_W,
	KEY_INPUT_S,
	KEY_INPUT_A,
	KEY_INPUT_D,
	KEY_INPUT_UP,
	KEY_INPUT_DOWN,
	KEY_INPUT_LEFT,
	KEY_INPUT_RIGHT,
	KEY_INPUT_C,
	KEY_INPUT_LSHIFT,
	InputManager::JOYPAD_NO::PAD1,
	InputManager::JOYPAD_BTN::R_BUMPER,
	InputManager::JOYPAD_BTN::DOWN
};

const Player::INPUT_CONFIG Player::PLAYER2_KEYBOARD_INPUT_CONFIG =
{
	INPUT_DEVICE::KEYBOARD,
	KEY_INPUT_I,
	KEY_INPUT_K,
	KEY_INPUT_J,
	KEY_INPUT_L,
	KEY_INPUT_NUMPAD8,
	KEY_INPUT_NUMPAD5,
	KEY_INPUT_NUMPAD4,
	KEY_INPUT_NUMPAD6,
	KEY_INPUT_NUMPAD7,
	KEY_INPUT_RSHIFT,
	InputManager::JOYPAD_NO::PAD1,
	InputManager::JOYPAD_BTN::R_BUMPER,
	InputManager::JOYPAD_BTN::DOWN
};

Player::Player(void)
	:
	ActorBase(),
	gravityVelocity_(0.0f),
	isInputEnabled_(true),
	cameraAngles_(VGet(0.0f, 0.0f, 0.0f)),
	hp_(HP_MAX),
	damageCooldownFrame_(0),
	animController_(nullptr),
	inputConfig_(KEYBOARD_INPUT_CONFIG),
	state_(STATE::IDLE)
{
}

Player::~Player(void)
{
	delete animController_;
}

void Player::Init(void)
{
	InitLoad();

	if (transform_.modelId != -1)
	{
		animController_ = new AnimationController(transform_.modelId);
	}

	InitTransform();
	InitCollider();
	InitAnimation();
	InitPost();

	ChangeState(STATE::IDLE, true);
}

void Player::Update(void)
{
	if (damageCooldownFrame_ > 0)
	{
		damageCooldownFrame_--;
	}

	UpdateCameraInput();
	UpdateMoveInput();

	ResolveWallCollision();
	ApplyGravity();
	ResolveWallCollision();

	UpdateState();

	switch (state_)
	{
	case Player::STATE::IDLE:
		UpdateIdle();
		break;
	case Player::STATE::WALK:
		UpdateWalk();
		break;
	case Player::STATE::RUN:
		UpdateRun();
		break;
	case Player::STATE::JUMP:
		UpdateJump();
		break;
	case Player::STATE::CROUCHED:
		UpdateCrouched();
		break;
	default:
		break;
	}

	transform_.Update();

	if (animController_ != nullptr)
	{
		animController_->Update();
	}
}

bool Player::IsKeyboardInputEnabled(void) const
{
	return inputConfig_.device == INPUT_DEVICE::KEYBOARD ||
		inputConfig_.device == INPUT_DEVICE::BOTH;
}

bool Player::IsPadInputEnabled(void) const
{
	return inputConfig_.device == INPUT_DEVICE::PAD ||
		inputConfig_.device == INPUT_DEVICE::BOTH;
}

VECTOR Player::GetMoveInputVector(void) const
{
	VECTOR inputDir = AsoUtility::VECTOR_ZERO;

	if (!isInputEnabled_)
	{
		return inputDir;
	}

	if (IsKeyboardInputEnabled())
	{
		if (CheckHitKey(inputConfig_.moveForwardKey))
		{
			inputDir.z += MOVE_SPEED;
		}
		if (CheckHitKey(inputConfig_.moveBackKey))
		{
			inputDir.z -= MOVE_SPEED;
		}
		if (CheckHitKey(inputConfig_.moveLeftKey))
		{
			inputDir.x -= MOVE_SPEED;
		}
		if (CheckHitKey(inputConfig_.moveRightKey))
		{
			inputDir.x += MOVE_SPEED;
		}
	}

	if (IsPadInputEnabled())
	{
		InputManager& input = InputManager::GetInstance();
		const auto padState = input.GetJPadInputState(inputConfig_.padNo);
		const VECTOR padDir = input.GetDirXZAKey(padState.AKeyLX, padState.AKeyLY);

		inputDir.x += padDir.x * MOVE_SPEED;
		inputDir.z += padDir.z * MOVE_SPEED;
	}

	return inputDir;
}

void Player::UpdateMoveInput(void)
{
	const VECTOR inputDir = GetMoveInputVector();

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

	if (IsKeyboardInputEnabled())
	{
		if (CheckHitKey(inputConfig_.cameraUpKey))
		{
			cameraAngles_.x -= CAMERA_ROT_SPEED;
		}
		if (CheckHitKey(inputConfig_.cameraDownKey))
		{
			cameraAngles_.x += CAMERA_ROT_SPEED;
		}
		if (CheckHitKey(inputConfig_.cameraLeftKey))
		{
			cameraAngles_.y -= CAMERA_ROT_SPEED;
		}
		if (CheckHitKey(inputConfig_.cameraRightKey))
		{
			cameraAngles_.y += CAMERA_ROT_SPEED;
		}
	}

	if (IsPadInputEnabled())
	{
		InputManager& input = InputManager::GetInstance();
		const auto padState = input.GetJPadInputState(inputConfig_.padNo);

		const float rx =
			static_cast<float>(padState.AKeyRX) / InputManager::AKEY_VAL_MAX;
		const float ry =
			static_cast<float>(padState.AKeyRY) / InputManager::AKEY_VAL_MAX;

		if (fabsf(rx) >= InputManager::THRESHOLD)
		{
			cameraAngles_.y += rx * CAMERA_ROT_SPEED * 2.0f;
		}
		if (fabsf(ry) >= InputManager::THRESHOLD)
		{
			cameraAngles_.x += ry * CAMERA_ROT_SPEED * 2.0f;
		}
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

bool Player::IsRunInput(void) const
{
	if (!isInputEnabled_)
	{
		return false;
	}

	bool isRun = false;

	if (IsKeyboardInputEnabled())
	{
		isRun = isRun || CheckHitKey(inputConfig_.runKey) || CheckHitKey(KEY_INPUT_RSHIFT);
	}

	if (IsPadInputEnabled())
	{
		const auto& input = InputManager::GetInstance();
		isRun = isRun || input.IsPadBtnNew(inputConfig_.padNo, inputConfig_.runPadBtn);
	}

	return isRun;
}

bool Player::IsCrouchInput(void) const
{
	if (!isInputEnabled_)
	{
		return false;
	}

	bool isCrouch = false;

	if (IsKeyboardInputEnabled())
	{
		isCrouch = isCrouch || CheckHitKey(inputConfig_.crouchKey);
	}

	if (IsPadInputEnabled())
	{
		const auto& input = InputManager::GetInstance();
		isCrouch = isCrouch || input.IsPadBtnNew(inputConfig_.padNo, inputConfig_.crouchPadBtn);
	}

	return isCrouch;
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

void Player::SetInputConfig(const INPUT_CONFIG& config)
{
	inputConfig_ = config;
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
	if (animController_ == nullptr)
	{
		return;
	}

	std::string path = Application::PATH_MODEL + "Player/Animation/";

	animController_->Add((int)ANIM_TYPE::IDLE, path + "Idle.mv1", 20.0f);
	animController_->Add((int)ANIM_TYPE::CROUCHED, path + "Crouched.mv1", 20.0f);
	animController_->Add((int)ANIM_TYPE::WALK, path + "Walking.mv1", 60.0f);
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

void Player::UpdateState(void)
{
	ChangeState(GetNextState());
}

Player::STATE Player::GetNextState(void) const
{
	VECTOR hitPos = AsoUtility::VECTOR_ZERO;

	if (!CheckGround(hitPos))
	{
		return STATE::JUMP;
	}

	if (!isInputEnabled_)
	{
		return STATE::IDLE;
	}

	if (IsCrouchInput())
	{
		return STATE::CROUCHED;
	}

	if (HasMoveInput())
	{
		if (IsRunInput())
		{
			return STATE::RUN;
		}

		return STATE::WALK;
	}

	return STATE::IDLE;
}

void Player::ChangeState(STATE newState, bool isForce)
{
	if (!isForce && state_ == newState)
	{
		return;
	}

	state_ = newState;

	switch (state_)
	{
	case STATE::IDLE:
		OnEnterIdle();
		break;
	case STATE::WALK:
		OnEnterWalk();
		break;
	case STATE::RUN:
		OnEnterRun();
		break;
	case STATE::JUMP:
		OnEnterJump();
		break;
	case STATE::CROUCHED:
		OnEnterCrouched();
		break;
	default:
		break;
	}
}

bool Player::HasMoveInput(void) const
{
	if (!isInputEnabled_)
	{
		return false;
	}

	return !AsoUtility::EqualsVZero(GetMoveInputVector());
}

void Player::UpdateIdle(void)
{
}

void Player::UpdateWalk(void)
{
}

void Player::UpdateRun(void)
{
}

void Player::UpdateJump(void)
{
}

void Player::UpdateCrouched(void)
{
}

void Player::OnEnterIdle(void)
{
	if (animController_ != nullptr)
	{
		animController_->Play((int)ANIM_TYPE::IDLE, true);
	}
}

void Player::OnEnterWalk(void)
{
	if (animController_ != nullptr)
	{
		animController_->Play((int)ANIM_TYPE::WALK, true);
	}
}

void Player::OnEnterRun(void)
{
	if (animController_ != nullptr)
	{
		animController_->Play((int)ANIM_TYPE::WALK, true);
	}
}

void Player::OnEnterJump(void)
{
	if (animController_ != nullptr)
	{
		animController_->Play((int)ANIM_TYPE::IDLE, true);
	}
}

void Player::OnEnterCrouched(void)
{
	if (animController_ != nullptr)
	{
		animController_->Play((int)ANIM_TYPE::CROUCHED, true);
	}
}

void Player::TakeDamage(int damage)
{
	if (damage <= 0)
	{
		return;
	}

	if (!CanTakeDamage())
	{
		return;
	}

	hp_ -= damage;
	if (hp_ < 0)
	{
		hp_ = 0;
	}

	damageCooldownFrame_ = DAMAGE_COOLDOWN_MAX;
}

bool Player::CanTakeDamage(void) const
{
	return hp_ > 0 && damageCooldownFrame_ <= 0;
}

bool Player::IsDead(void) const
{
	return hp_ <= 0;
}

int Player::GetHp(void) const
{
	return hp_;
}

int Player::GetHpMax(void) const
{
	return HP_MAX;
}

float Player::GetHpRate(void) const
{
	if (HP_MAX <= 0)
	{
		return 0.0f;
	}

	return static_cast<float>(hp_) / static_cast<float>(HP_MAX);
}

