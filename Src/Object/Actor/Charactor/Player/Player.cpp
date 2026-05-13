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
	isInputEnabled_(true)
{
}

Player::~Player(void)
{
}

void Player::Update(void)
{
	// 簡易的な移動処理
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
		const float yaw = SceneManager::GetInstance().GetCamera()->GetAngles().y;
		VECTOR moveDir = AsoUtility::VNormalize(inputDir);
		moveDir = VTransform(moveDir, MGetRotY(yaw));
		moveDir.y = 0.0f;
		transform_.pos = VAdd(transform_.pos, VScale(moveDir, moveSpeed));
	}

	ResolveWallCollision();
	ApplyGravity();
	ResolveWallCollision();
	transform_.Update();
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

	// 描画されているかチェック
	if (transform_.modelId == -1)
	{
		// ロード失敗
		return;
	}
}

void Player::InitTransform(void)
{
	transform_.scl = { 0.5f,0.5f,0.5f };
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Identity();
	transform_.pos = INIT_POS;
	transform_.Update();
}

void Player::InitCollider(void)
{
	// 主に地面との衝突で使用する線分コライダ
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::LINE), colLine);

	// 主に壁や木などの衝突で使用するカプセルコライダ
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
	// 初期座標に移動
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

