#include "Subject.h"
#include <cmath>
#include <DxLib.h>
#include "../../../../Manager/Camera.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Object/Collider/ColliderModel.h"
#include "../../../../Object/Collider/ColliderCapsule.h"
#include "../../../../Object/Collider/ColliderLine.h"
#include "../../../../Utility/AsoUtility.h"

Subject::Subject(void)
	:
	ActorBase(),
	gravityVelocity_(0.0f),
	isInoputEnabled_(true),
	knockBackVelocity_(VGet(0, 0, 0)),
	moveAreaMin_(VGet(-500.0f, 0.0f, -500.0f)),
	moveAreaMax_(VGet(500.0f, 0.0f, 500.0f)),
	moveDir_(VGet(0.0f, 0.0f, 1.0f)),
	moveDirChangeFrame_(0),
	baseScale_(VGet(0.01f, 0.01f, 0.01f)),
	actionState_(ACTION_STATE::MOVE),
	attackCooldownFrame_(0),
	attackFrame_(0),
	attackTargetPos_(VGet(0.0f, 0.0f, 0.0f)),
	isAttackHitPending_(false),
	playerDetectRange_(DETECTION_RANGE)
{
	// 初期化はActorBaseのInitで行う
}

Subject::~Subject(void)
{
}

void Subject::Update(void)
{
	//死亡演出中は移動／攻撃処理を行わず、ノックバックと重力のみ適用してフレームを消費する
	if (isDying_)
	{
		dyingFrame_++;

		transform_.pos =
			VAdd(transform_.pos,
				knockBackVelocity_);

		knockBackVelocity_ =
			VScale(knockBackVelocity_, 0.9f);

		ApplyGravity();

		transform_.Update();
		return;
	}

	// 攻撃クールダウンのフレームを減算
	if (attackCooldownFrame_ > 0)
	{
		attackCooldownFrame_--;
	}

	const VECTOR prevPos = transform_.pos;

	// 追加: スタン中は移動／攻撃処理を行わず、重力のみ適用してフレームを消費する
	if (stunFrames_ > 0)
	{
		stunFrames_--;
		// スタン中は状態を MOVE に固定して攻撃をキャンセル済みのはずだが念のため調整
		actionState_ = ACTION_STATE::MOVE;
		attackFrame_ = 0;
		isAttackHitPending_ = false;

		ApplyGravity();
		transform_.Update();
		return;
	}

	if (actionState_ == ACTION_STATE::ATTACK)
	{
		UpdateAttack();
		ApplyGravity();
		transform_.Update();
		return;
	}

	UpdateMove();
	ClampToMoveArea();
	ResolveWallCollision(prevPos);
	ApplyGravity();
	transform_.Update();
}

void Subject::SetPos(const VECTOR& pos)
{
	transform_.pos = pos;
	transform_.Update();
}

void Subject::SetInputEnabled(bool isEnabled)
{
	isInoputEnabled_ = isEnabled;
}


void Subject::SetMoveArea(const VECTOR& minPos, const VECTOR& maxPos)
{
	moveAreaMin_ = minPos;
	moveAreaMax_ = maxPos;
	ClampToMoveArea();
}

void Subject::Draw(void)
{
	// 死亡演出中は点滅
	if (isDying_)
	{
		if ((dyingFrame_ / 5) % 2 == 0)
		{
			return;
		}
	}

	ActorBase::Draw();

	// スタン状態の視覚表示
	if (stunFrames_ > 0)
	{
		DrawStunEffect();
	}
}

void Subject::DrawStunEffect(void)
{
	// 敵の頭上の位置を計算
	VECTOR headPos = VAdd(transform_.pos, VGet(0.0f, 140.0f, 0.0f));

	// 3D座標を2D画面座標に変換
	VECTOR screenPos = ConvWorldPosToScreenPos(headPos);

	// 画面外の場合は描画しない
	if (screenPos.z < 0.0f || screenPos.z > 1.0f)
	{
		return;
	}

	int centerX = static_cast<int>(screenPos.x);
	int centerY = static_cast<int>(screenPos.y);

	// 回転する星マークを描画（3つの星を円周上に配置）
	const int starCount = 3;
	const float radius = 30.0f;
	const float rotSpeed = 0.1f;
	float angle = static_cast<float>(GetNowCount()) * rotSpeed;

	for (int i = 0; i < starCount; ++i)
	{
		float currentAngle = angle + (DX_PI_F * 2.0f / starCount) * i;
		int starX = centerX + static_cast<int>(cos(currentAngle) * radius);
		int starY = centerY + static_cast<int>(sin(currentAngle) * radius);

		// 星を描画（5角形の星）
		DrawStar(starX, starY, 8, GetColor(255, 255, 100));
	}

	// スタン時間のテキスト表示
	const int textColor = GetColor(255, 200, 50);
	const int backColor = GetColor(0, 0, 0);

	// 背景を半透明で描画
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	DrawBox(centerX - 30, centerY + 40, centerX + 30, centerY + 60, backColor, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// スタンテキスト表示
	DrawFormatString(
		centerX - 25,
		centerY + 43,
		textColor,
		"STUN!");
}

void Subject::DrawStar(int x, int y, int size, unsigned int color)
{
	// 5角形の星を描画
	const int points = 10; // 外側5点 + 内側5点
	const float outerRadius = static_cast<float>(size);
	const float innerRadius = outerRadius * 0.4f;

	Vector2 vertices[10];

	for (int i = 0; i < points; ++i)
	{
		float angle = (DX_PI_F * 2.0f / points) * i - DX_PI_F / 2.0f;
		float radius = (i % 2 == 0) ? outerRadius : innerRadius;

		vertices[i].x = x + cos(angle) * radius;
		vertices[i].y = y + sin(angle) * radius;
	}

	// 星の各三角形を描画
	for (int i = 0; i < points; ++i)
	{
		int next = (i + 1) % points;
		DrawTriangle(
			static_cast<int>(vertices[i].x),
			static_cast<int>(vertices[i].y),
			static_cast<int>(vertices[next].x),
			static_cast<int>(vertices[next].y),
			x,
			y,
			color,
			TRUE
		);
	}

	// 星の輪郭を描画
	for (int i = 0; i < points; ++i)
	{
		int next = (i + 1) % points;
		DrawLine(
			static_cast<int>(vertices[i].x),
			static_cast<int>(vertices[i].y),
			static_cast<int>(vertices[next].x),
			static_cast<int>(vertices[next].y),
			GetColor(255, 255, 0),
			2
		);
	}
}

void Subject::StartDying()
{

	

	if (isDying_)
	{
		return;
	}

	isDying_ = true;
	dyingFrame_ = 0;

	// 動きを止める
	stunFrames_ = DYING_FRAME_MAX;

	// 攻撃中なら解除
	actionState_ = ACTION_STATE::MOVE;
	attackFrame_ = 0;
	isAttackHitPending_ = false;
}

void Subject::InitLoad(void)
{
	transform_.SetModel(
		resMng_.LoadModelDuplicate(GetModelType()));

	// 描画されているかチェック
	if (transform_.modelId == -1)
	{
		// ロード失敗
		return;
	}
}

void Subject::InitTransform(void)
{
	transform_.scl = { 0.01f, 0.01f, 0.01f };
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Identity();
	transform_.pos = INIT_POS;
	transform_.Update();
}

void Subject::InitCollider(void)
{
	// 主に地面との衝突で使用する線分コライダ
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::STAGE, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::LINE), colLine);

	// 主に壁との衝突で使用するカプセルコライダ
	ColliderCapsule* colCapsule = new ColliderCapsule(
		ColliderBase::TAG::STAGE, &transform_,
		COL_CAPSULE_TOP_LOCAL_POS, COL_CAPSULE_DOWN_LOCAL_POS, COL_CAPSULE_RADIUS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);
}

void Subject::InitAnimation(void)
{
}

void Subject::InitPost(void)
{
	// サブジェクトの初期座標を設定
	transform_.pos = GetInitPos();
	PickRandomMoveDirection();
}


void Subject::ApplyGravity(void)
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

bool Subject::CheckGround(VECTOR& hitPos) const
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

void Subject::ResolveWallCollision(const VECTOR& prevPos)
{
	VECTOR hitPos = AsoUtility::VECTOR_ZERO;

	const VECTOR xMovedPos = VGet(transform_.pos.x, prevPos.y, prevPos.z);
	if (CheckWallSegment(
		VAdd(prevPos, VGet(0.0f, WALL_CHECK_HEIGHT, 0.0f)),
		VAdd(xMovedPos, VGet(0.0f, WALL_CHECK_HEIGHT, 0.0f)),
		hitPos))
	{
		transform_.pos.x = prevPos.x;
		PickRandomMoveDirection();
	}

	const VECTOR zStartPos = VGet(transform_.pos.x, prevPos.y, prevPos.z);
	const VECTOR zMovedPos = VGet(transform_.pos.x, prevPos.y, transform_.pos.z);
	if (CheckWallSegment(
		VAdd(zStartPos, VGet(0.0f, WALL_CHECK_HEIGHT, 0.0f)),
		VAdd(zMovedPos, VGet(0.0f, WALL_CHECK_HEIGHT, 0.0f)),
		hitPos))
	{
		transform_.pos.z = prevPos.z;
		PickRandomMoveDirection();
	}
}

bool Subject::CheckWallSegment(const VECTOR& start, const VECTOR& end, VECTOR& hitPos) const
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

void Subject::UpdateMove(void)
{

	if (VSize(knockBackVelocity_) > 0.1f)
	{
		transform_.pos =
			VAdd(transform_.pos,
				knockBackVelocity_);

		knockBackVelocity_ =
			VScale(knockBackVelocity_, 0.9f);
	}

	if (moveDirChangeFrame_ <= 0)
	{
		PickRandomMoveDirection();
	}

	transform_.pos = VAdd(transform_.pos, VScale(moveDir_, MOVE_SPEED));
	moveDirChangeFrame_--;
}

ResourceManager::SRC Subject::GetModelType() const
{
	// デフォルトではSUBJECTを返すが、派生クラスでオーバーライドして異なるモデルタイプを返すことができる
	return ResourceManager::SRC::SUBJECT;
}

void Subject::SetPlayerPos(const std::vector<VECTOR>& positions)
{
	playerPos_ = positions;
}

void Subject::SetPlayerDetectRange(float range)
{
	if (range < 0.0f) return;
	playerDetectRange_ = range;
}

void Subject::PickRandomMoveDirection(void)
{
	const float angle = static_cast<float>(GetRand(359)) * DX_PI_F / 180.0f;
	moveDir_ = VGet(cosf(angle), 0.0f, sinf(angle));

	moveDirChangeFrame_ =
		RANDOM_DIR_CHANGE_MIN +
		GetRand(RANDOM_DIR_CHANGE_MAX - RANDOM_DIR_CHANGE_MIN);

	FaceMoveDirection();
}

void Subject::FaceMoveDirection(void)
{
	if (AsoUtility::EqualsVZero(moveDir_))
	{
		return;
	}

	transform_.quaRot = Quaternion::LookRotation(moveDir_);
}

void Subject::ClampToMoveArea(void)
{

	//printfDx("Clamp Before : %f\n", transform_.pos.x);

	bool isOut = false;

	if (transform_.pos.x < moveAreaMin_.x)
	{
		transform_.pos.x = moveAreaMin_.x;
		isOut = true;
	}
	else if (transform_.pos.x > moveAreaMax_.x)
	{
		transform_.pos.x = moveAreaMax_.x;
		isOut = true;
	}

	if (transform_.pos.z < moveAreaMin_.z)
	{
		transform_.pos.z = moveAreaMin_.z;
		isOut = true;
	}
	else if (transform_.pos.z > moveAreaMax_.z)
	{
		transform_.pos.z = moveAreaMax_.z;
		isOut = true;
	}

	if (!isOut)
	{
		return;
	}

	const VECTOR center = VScale(VAdd(moveAreaMin_, moveAreaMax_), 0.5f);
	VECTOR toCenter = VSub(center, transform_.pos);
	toCenter.y = 0.0f;

	if (!AsoUtility::EqualsVZero(toCenter))
	{
		moveDir_ = AsoUtility::VNormalize(toCenter);
		moveDirChangeFrame_ =
			RANDOM_DIR_CHANGE_MIN +
			GetRand(RANDOM_DIR_CHANGE_MAX - RANDOM_DIR_CHANGE_MIN);
		FaceMoveDirection();
	}

	//printfDx("Clamp After : %f\n", transform_.pos.x);
}

bool Subject::IsInAttackRange(const VECTOR& targetPos) const
{
	VECTOR diff = VSub(targetPos, transform_.pos);
	diff.y = 0.0f;
	return VSize(diff) <= ATTACK_RANGE;
}

bool Subject::CanStartAttack(void) const
{
	return actionState_ != ACTION_STATE::ATTACK && attackCooldownFrame_ <= 0;
}

bool Subject::StartAttack(const VECTOR& targetPos)
{
	if (!CanStartAttack())
	{
		return false;
	}

	actionState_ = ACTION_STATE::ATTACK;
	attackFrame_ = 0;
	attackTargetPos_ = targetPos;
	isAttackHitPending_ = false;
	transform_.scl = baseScale_;
	FaceTarget(targetPos);
	return true;
}

bool Subject::ConsumeAttackHit(void)
{
	if (!isAttackHitPending_)
	{
		return false;
	}

	isAttackHitPending_ = false;
	return true;
}

void Subject::Stun(int frames)
{
	if (frames <= 0) return;
	stunFrames_ = frames;
	// optionally reset action state
	actionState_ = ACTION_STATE::MOVE;
	attackFrame_ = 0;
	isAttackHitPending_ = false;
}

void Subject::AddKnockBack(const VECTOR& force)
{
	knockBackVelocity_ = force;
}

void Subject::UpdateAttack(void)
{
	attackFrame_++;
	FaceTarget(attackTargetPos_);

	float t = static_cast<float>(attackFrame_) / static_cast<float>(ATTACK_FRAME_MAX);
	if (t < 0.0f)
	{
		t = 0.0f;
	}
	if (t > 1.0f)
	{
		t = 1.0f;
	}

	float stretch = 0.0f;
	if (t < 0.5f)
	{
		stretch = (t / 0.5f) * ATTACK_STRETCH_MAX;
	}
	else
	{
		stretch = ((1.0f - t) / 0.5f) * ATTACK_STRETCH_MAX;
	}

	transform_.scl = VGet(
		baseScale_.x * (1.0f + stretch),
		baseScale_.y * (1.0f - stretch * 0.35f),
		baseScale_.z * (1.0f + stretch * 1.5f));

	if (attackFrame_ == ATTACK_HIT_FRAME)
	{
		isAttackHitPending_ = true;
	}

	if (attackFrame_ >= ATTACK_FRAME_MAX)
	{
		actionState_ = ACTION_STATE::MOVE;
		attackCooldownFrame_ = ATTACK_COOLDOWN_MAX;
		attackFrame_ = 0;
		transform_.scl = baseScale_;
		moveDirChangeFrame_ = 0;
	}
}

void Subject::FaceTarget(const VECTOR& targetPos)
{
	VECTOR dir = VSub(targetPos, transform_.pos);
	dir.y = 0.0f;

	if (AsoUtility::EqualsVZero(dir))
	{
		return;
	}

	transform_.quaRot = Quaternion::LookRotation(AsoUtility::VNormalize(dir));
}