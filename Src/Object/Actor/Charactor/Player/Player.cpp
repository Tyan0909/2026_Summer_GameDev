#include "Player.h"
#include "../../../../Manager/Camera.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Object/Collider/ColliderModel.h"
#include "../../../../Object/Collider/ColliderCapsule.h"
#include "../../../../Object/Collider/ColliderLine.h"
#include "../../../../Object/Collider/ColliderCapsule.h"

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
	const VECTOR prevPos = transform_.pos;

	// 簡易的な移動処理
	VECTOR inputDir = AsoUtility::VECTOR_ZERO;
	if(isInputEnabled_ && CheckHitKey(KEY_INPUT_W))
	{
		inputDir.z += 1.0f;
	}
	if(isInputEnabled_ && CheckHitKey(KEY_INPUT_S))
	{
		inputDir.z -= 1.0f;
	}
	if(isInputEnabled_ && CheckHitKey(KEY_INPUT_A))
	{
		inputDir.x -= 1.0f;
	}
	if(isInputEnabled_ && CheckHitKey(KEY_INPUT_D))
	{
		inputDir.x += 1.0f;
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

	ResolveWallCollision(prevPos);
	ApplyGravity();
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
	transform_.scl = { 1.0f, 1.0f, 1.0f };
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Identity();
	transform_.pos = INIT_POS;
	transform_.Update();
}

void Player::InitCollider(void)
{
	//// DxLib側の衝突判定をセットアップ
	//MV1SetupCollInfo(transform_.modelId);
	//
	//// モデルのコライダー
	//ColliderModel* colModel =
	//	new ColliderModel(ColliderBase::TAG::STAGE, &transform_);

	//ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::MODEL), colModel);


		// 主に地面との衝突で仕様する線分コライダ
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::LINE), colLine);

	// 主に壁や木などの衝突で仕様するカプセルコライダ
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
			return true;
		}
	}

	return false;
}

void Player::ResolveWallCollision(const VECTOR& prevPos)
{
	VECTOR hitPos = AsoUtility::VECTOR_ZERO;

	const VECTOR xMovedPos = VGet(transform_.pos.x, prevPos.y, prevPos.z);
	if (CheckWallSegment(
		VAdd(prevPos, VGet(0.0f, WALL_CHECK_HEIGHT, 0.0f)),
		VAdd(xMovedPos, VGet(0.0f, WALL_CHECK_HEIGHT, 0.0f)),
		hitPos))
	{
		transform_.pos.x = prevPos.x;
	}

	const VECTOR zStartPos = VGet(transform_.pos.x, prevPos.y, prevPos.z);
	const VECTOR zMovedPos = VGet(transform_.pos.x, prevPos.y, transform_.pos.z);
	if (CheckWallSegment(
		VAdd(zStartPos, VGet(0.0f, WALL_CHECK_HEIGHT, 0.0f)),
		VAdd(zMovedPos, VGet(0.0f, WALL_CHECK_HEIGHT, 0.0f)),
		hitPos))
	{
		transform_.pos.z = prevPos.z;
	}
}

bool Player::CheckWallSegment(const VECTOR& start, const VECTOR& end, VECTOR& hitPos) const
{
	const VECTOR move = VSub(end, start);
	if (AsoUtility::EqualsVZero(move))
	{
		return false;
	}

	const VECTOR dir = AsoUtility::VNormalize(move);
	const VECTOR checkEnd = VAdd(end, VScale(dir, WALL_PUSH_BACK));

	for (const auto& hitCollider : hitColliders_)
	{
		if (hitCollider == nullptr ||
			hitCollider->GetShape() != ColliderBase::SHAPE::MODEL)
		{
			continue;
		}

		const auto* modelCollider = static_cast<const ColliderModel*>(hitCollider);
		auto hit = modelCollider->GetNearestHitPolyLine(start, checkEnd, true);
		if (hit.HitFlag)
		{
			hitPos = hit.HitPosition;
			return true;
		}
	}

	return false;
}

