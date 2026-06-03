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
#include "../../../../Scene/BuySelect.h" // ITEM_TYPE を参照

// TPS_CAMERA_LOCAL_POS の実体定義（クラス宣言はヘッダにあり）
const VECTOR Player::TPS_CAMERA_LOCAL_POS = { 0.0f, 60.0f, -180.0f };

// 使用アイテム順序の定義（フラグ、スパイク、地雷）
const std::vector<ITEM_TYPE> Player::usableOrder_ = {
	ITEM_TYPE::FRAG_GRENADE,
	ITEM_TYPE::SPIKE_TRAP,
	ITEM_TYPE::EXPLOSIVE_TRAP
};

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
	// 変更: 2P も LSHIFT で走るようにする
	KEY_INPUT_LSHIFT,
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
	state_(STATE::IDLE),
	selectedUsableIndex_(-1), // 変更: 未選択は -1
	spikeTrapCount_(0),
	explosiveTrapCount_(0),
	fragGrenadeCount_(0),
	helmetUsesRemaining_(0)
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
		// フレーム時間に依存させて MOVE_SPEED を実際の速度に使う
		float delta = SceneManager::GetInstance().GetDeltaTime();
		if (delta <= 0.0f) delta = 1.0f / 60.0f;

		// 基本速度
		float speed = MOVE_SPEED * delta;

		// 走り入力なら倍率を掛ける
		if (IsRunInput())
		{
			const float RUN_MULT =2.25f;
			speed *= RUN_MULT;
		}

		// しゃがみ入力なら減速
		if (IsCrouchInput())
		{
			const float CROUCH_MULT = 0.5f;
			speed *= CROUCH_MULT;
		}

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

		transform_.pos = VAdd(transform_.pos, VScale(moveDir, speed));
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
		// 入力設定の runKey をチェック + 左右SHIFT の両方を許容する
		isRun = isRun || CheckHitKey(inputConfig_.runKey) || CheckHitKey(KEY_INPUT_LSHIFT) || CheckHitKey(KEY_INPUT_RSHIFT);
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
	// class の static メンバを明示的に参照する
	VECTOR cameraOffset = Player::TPS_CAMERA_LOCAL_POS;
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
		// RUN アニメがない場合は WALK を流用（必要なら専用アニメを追加して差し替えてください）
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

// 追加実装: アイテム付与
void Player::AddItem(int itemType)
{
	// 重複登録は inventory に保持する（必要ならユニーク化する）
	inventory_.push_back(itemType);

	switch (static_cast<ITEM_TYPE>(itemType))
	{
	case ITEM_TYPE::HELMET:
		// ヘルメットは購入で合計 3 回分の防御を付与（仕様）
		helmetUsesRemaining_ = 3;
		break;
	case ITEM_TYPE::INSURANCE_CAMERA:
		hasInsurance_ = true;
		break;
	case ITEM_TYPE::ZOOM_CAMERA:
		hasZoomCamera_ = true;
		break;
	case ITEM_TYPE::SPIKE_TRAP:
		++spikeTrapCount_;
		break;
	case ITEM_TYPE::EXPLOSIVE_TRAP:
		++explosiveTrapCount_;
		break;
	case ITEM_TYPE::FRAG_GRENADE:
		++fragGrenadeCount_;
		break;
	default:
		// 他アイテムは現状 inventory に保持するのみ
		break;
	}
}

// スパイク使用（設置要求）：在庫があればデクリメントして true
bool Player::UseSpikeTrap()
{
	if (spikeTrapCount_ > 0)
	{
		--spikeTrapCount_;
		return true;
	}
	return false;
}

// 地雷使用（設置要求）：在庫があればデクリメントして true
bool Player::UseExplosiveTrap()
{
	if (explosiveTrapCount_ > 0)
	{
		--explosiveTrapCount_;
		return true;
	}
	return false;
}

// フラググレネード使用（消費）
bool Player::UseFragGrenade()
{
	if (fragGrenadeCount_ > 0)
	{
		--fragGrenadeCount_;
		return true;
	}
	return false;
}

int Player::GetSpikeCount() const { return spikeTrapCount_; }
int Player::GetMineCount() const { return explosiveTrapCount_; }
int Player::GetFragCount() const { return fragGrenadeCount_; }
int Player::GetHelmetUses() const { return helmetUsesRemaining_; }

// 選択アイテム操作 — 所持しているものだけを順送り
void Player::CycleSelectedUsableItem(int dir)
{
	if (usableOrder_.empty())
	{
		selectedUsableIndex_ = -1;
		return;
	}

	const int len = static_cast<int>(usableOrder_.size());

	auto HasItem = [this](ITEM_TYPE t) -> bool {
		switch (t)
		{
		case ITEM_TYPE::FRAG_GRENADE: return fragGrenadeCount_ > 0;
		case ITEM_TYPE::SPIKE_TRAP: return spikeTrapCount_ > 0;
		case ITEM_TYPE::EXPLOSIVE_TRAP: return explosiveTrapCount_ > 0;
		default: return false;
		}
	};

	// dir == 0 : 最初に所持しているアイテムを選ぶ（自動選択用）
	if (dir == 0)
	{
		for (int i = 0; i < len; ++i)
		{
			if (HasItem(usableOrder_[i]))
			{
				selectedUsableIndex_ = i;
				return;
			}
		}
		selectedUsableIndex_ = -1;
		return;
	}

	// dir != 0 の場合、未選択ならまず「最初に所持しているアイテム」を選ぶ
	if (selectedUsableIndex_ < 0)
	{
		for (int i = 0; i < len; ++i)
		{
			if (HasItem(usableOrder_[i]))
			{
				selectedUsableIndex_ = i;
				return;
			}
		}
		// 所持アイテムが無ければ何もしない
		return;
	}

	// 通常の巡回処理（現在選択から次を探す）
	int start = selectedUsableIndex_;

	for (int step = 1; step <= len; ++step)
	{
		int idx = (start + dir * step) % len;
		if (idx < 0) idx += len;
		if (HasItem(usableOrder_[idx]))
		{
			selectedUsableIndex_ = idx;
			return;
		}
	}

	// 見つからなければ未選択へ
	selectedUsableIndex_ = -1;
}

// 選択中アイテム取得（未選択なら NORMAL_CAMERA を返す）
ITEM_TYPE Player::GetSelectedUsableItemType() const
{
	if (usableOrder_.empty()) return ITEM_TYPE::NORMAL_CAMERA;
	if (selectedUsableIndex_ < 0 || selectedUsableIndex_ >= static_cast<int>(usableOrder_.size()))
	{
		return ITEM_TYPE::NORMAL_CAMERA;
	}
	return usableOrder_[selectedUsableIndex_];
}

// TakeDamage をヘルメット仕様に合わせて修正（既存メソッドを置換）
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

	// ヘルメットが残っていれば、その攻撃を無効化してヘルメット残数を減らす
	if (helmetUsesRemaining_ > 0)
	{
		--helmetUsesRemaining_;
		// ダメージ無効化（被ダメ無効だがクールダウンは設定）
		damageCooldownFrame_ = DAMAGE_COOLDOWN_MAX;
		return;
	}

	// 通常ダメージ処理
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

