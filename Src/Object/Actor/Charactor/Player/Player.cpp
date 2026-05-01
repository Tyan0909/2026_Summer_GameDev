<<<<<<< HEAD
#include "Player.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Object/Collider/ColliderModel.h"
#include "../../../../Object/Collider/ColliderCapsule.h"
=======
#include <string>
#include "../../../../Application.h"
>>>>>>> 8102eb1376e5a7f230c3d9cdaff2b637efd3dea8
#include "../../../../Utility/AsoUtility.h"
#include "../../../../Manager/InputManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/Camera.h"
#include "../../../Common/AnimationController.h"
#include "../../../Common/Capsule.h"
#include "Player.h"

Player::Player(void)
<<<<<<< HEAD
	:
	ActorBase(),
	gravityVelocity_(0.0f)
=======
>>>>>>> 8102eb1376e5a7f230c3d9cdaff2b637efd3dea8
{
	animCtrl_ = nullptr;

	// デバッグで初期は歩き
	state_ = STATE::WALK;

	// この辺は後で定数化する
	speed_ = 0.0f;
	moveDir_ = VGet(0.0f, 0.0f, 0.0f);
	movePow_ = VGet(0.0f, 0.0f, 0.0f);
	movedPos_ = VGet(0.0f, 0.0f, 0.0f);

	playerRotY_ = Quaternion();
	goalQuaRot_ = Quaternion();
	stepRotTime_ = 0.0f;

	jumpPow_ = VGet(0.0f, 0.0f, 0.0f);
	isJump_ = false;
	stepJump_ = 0.0f;

	// 衝突判定
	gravHitPosDown_ = VGet(0.0f, 0.0f, 0.0f);
	gravHitPosUp_ = VGet(0.0f, 0.0f, 0.0f);

	imgShadow_ = -1;

	capsule_ = nullptr;
}

Player::~Player(void)
{

}

void Player::Init(void)
{
	// モデルの基本設定
	transform_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::PLAYER));
	transform_.scl = VGet(1.0f, 1.0f, 1.0f);
	transform_.pos = VGet(0.0f, 100.0f, 0.0f);
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = 
		Quaternion::Euler({ 0.0f,AsoUtility::Deg2RadF(180.0f),0.0f });
	transform_.Update();

	// アニメーション
	InitAnimation();

	// カプセルコライダー
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_ ->SetLocalPosTop(VGet(0.0f, 100.0f, 0.0f));
	capsule_->SetLocalPosDown(VGet(0.0f, 30.0f, 0.0f));
	capsule_->SetRadius(20.0f);

	// 丸影 後で実装
	imgShadow_ = -1;

	// 初期状態
	ChangeState(STATE::IDLE);
}

void Player::Update(void)
{
<<<<<<< HEAD
	const VECTOR prevPos = transform_.pos;

	// 簡易的な移動処理
	if(CheckHitKey(KEY_INPUT_W))
	{
		transform_.pos.z += 1.0f;
	}
	if(CheckHitKey(KEY_INPUT_S))
	{
		transform_.pos.z -= 1.0f;
	}
	if(CheckHitKey(KEY_INPUT_A))
	{
		transform_.pos.x -= 1.0f;
	}
	if(CheckHitKey(KEY_INPUT_D))
	{
		transform_.pos.x += 1.0f;
	}

	ResolveWallCollision(prevPos);
	ApplyGravity();
	transform_.Update();
}

void Player::InitLoad(void)
{
	transform_.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::PLAYER));

	// 描画されているかチェック
	if (transform_.modelId == -1)
=======
	// 状態遷移
	switch (state_)
>>>>>>> 8102eb1376e5a7f230c3d9cdaff2b637efd3dea8
	{
	case STATE::IDLE:
		UpdateIdle();
		break;
	case STATE::WALK:
		UpdateWalk();
		break;
	case STATE::TAKE:
		UpdateTake();
		break;
	case STATE::JUMP:
		UpdateJump();
		break;
	default:
		break;
	}

	// モデルの基本情報更新
	transform_.Update();

	// アニメーション更新
	animCtrl_->Update();
}

void Player::Draw(void)
{
	// 描画順は要注意

	// 丸影の描画
	DrawShadow();

	// モデルの描画
	MV1DrawModel(transform_.modelId);
}

void Player::AddCollider(Collider* collider)
{
	colliders_.push_back(collider);
}

void Player::ClearCollider(void)
{
	colliders_.clear();
}

const Capsule* Player::GetCapsule(void) const
{
	return capsule_.get();
}

void Player::InitAnimation(void)
{
	// 挙動モデルが実装でき次第
	/*animCtrl_ = std::make_unique<AnimationController>(transform_.modelId);
	animCtrl_->AddInFbx(static_cast<int>(ANIM_TYPE::IDLE), 1.0f, 0);
	animCtrl_->AddInFbx(static_cast<int>(ANIM_TYPE::WALK), 1.0f, 1);
	animCtrl_->AddInFbx(static_cast<int>(ANIM_TYPE::TAKE), 1.0f, 2);
	animCtrl_->AddInFbx(static_cast<int>(ANIM_TYPE::JUMP), 1.0f, 3);*/
}

void Player::ChangeState(STATE state)
{
	if (state_ == state)
	{
		ChangeStateNone();
		return;
	}
	state_ = state;
	switch (state_)
	{
	case STATE::IDLE:
		ChangeStateIdle();
		break;
	case STATE::WALK:
		ChangeStateWalk();
		break;
	case STATE::TAKE:
		ChangeStateTake();
		break;
	case STATE::JUMP:
		ChangeStateJump();
		break;
	default:
		break;
	}
}

void Player::ChangeStateNone(void)
{
	// 状態遷移なし
}

void Player::ChangeStateIdle(void)
{
	// FBXファイルがない状態で実行すると例外スローが発生する
	// 待機状態
	//animCtrl_->Play(static_cast<int>(ANIM_TYPE::IDLE));
}

void Player::ChangeStateWalk(void)
{
	// 歩き状態

}

void Player::ChangeStateTake(void)
{
	// 写真撮影状態
}

void Player::ChangeStateJump(void)
{
	// ジャンプ状態
	// 多分Walkと状態遷移統合すると思う
}

void Player::UpdateNone(void)
{
	// 状態遷移なし
}

void Player::UpdateIdle(void)
{
	// 待機状態
}

void Player::UpdateWalk(void)
{
	// 移動処理
	ProcessMove();

	// ジャンプ処理
	ProcessJump();

	// 移動方向に応じた回転
	Rotate();

	// 重力による移動量
	CalcGravityPow();

	// 衝突判定
	Collision();

	// 回転させる
	transform_.quaRot = playerRotY_;
}

void Player::UpdateTake(void)
{
	// 写真撮影状態
}

void Player::UpdateJump(void)
{
	// ジャンプ状態
	/*ProcessMove();
	ProcessJump();
	Collision();
	Rotate();*/
}

void Player::DrawShadow(void)
{
	// 丸影の描画

	// 余裕があれば
}

void Player::ProcessMove(void)
{
	// 移動処理

	// InputManagerを取得
	auto& 
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

