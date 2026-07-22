#include "SubjectA.h"
#include "../../../Common/Transform.h"
#include "../../Charactor/Player/Player.h"
#include "../../../../Utility/AsoUtility.h"
#include <cfloat>
#include <cmath>

SubjectA::SubjectA(void)
	:
	player_(nullptr),
	hoverBaseY_(0.0f),
	hoverAmplitude_(10.0f), // ボブの振幅（任意調整）
	hoverSpeed_(0.08f),     // ボブの速度（任意調整）
	hoverAngle_(0.0f)
{
	// Player を生成しない（外部から座標が渡される前提）
}

SubjectA::~SubjectA(void)
{
}

void SubjectA::InitPost(void)
{
	Subject::InitPost();

	// SubjectA の移動範囲設定
	SetMoveArea(VGet(-40000.0f, 0.0f, -40000.0f),
		VGet(40000.0f, 0.0f, 40000.0f));

	// 浮遊をもっと高くする（基準高度を現在位置より大きくオフセット）
	// 必要に応じてオフセット値を調整してください
	constexpr float HOVER_HEIGHT_OFFSET = 900.0f;
	hoverBaseY_ = transform_.pos.y + HOVER_HEIGHT_OFFSET;
	hoverAngle_ = 0.0f;
}

void SubjectA::InitLoad(void)
{
	// SubjectA のモデルロード
	Subject::InitLoad();

	//　描画されているかどうか確認
	if (transform_.modelId < 0)
	{
		// モデル
		transform_.SetModel(resMng_.LoadModelDuplicate(GetModelType()));
	}
}

void SubjectA::InitTransform(void)
{
	transform_.scl = { 5.01f, 5.01f, 5.01f };
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Identity();
	transform_.pos = INIT_POS;
	transform_.Update();
}

VECTOR SubjectA::GetInitPos(void)
{
	return VGet(0.0f, 100.0f, 0.0f);
}

ResourceManager::SRC SubjectA::GetModelType() const
{
	return ResourceManager::SRC::ENEMY_UFO;
}

void SubjectA::UpdateMove(void)
{
	// 攻撃・プレイヤー追従は行わず、水平にゆっくり漂う挙動
	if (moveDirChangeFrame_ <= 0)
	{
		PickRandomMoveDirection();
	}

	// 少し遅めの移動速度にしてふわふわ感を出す
	const float driftSpeed = MOVE_SPEED * 0.6f;
	transform_.pos = VAdd(transform_.pos, VScale(moveDir_, driftSpeed));
	moveDirChangeFrame_--;
	FaceMoveDirection();

	// 
}

// 重力無視でホバリング
void SubjectA::ApplyGravity(void)
{
	// 基準高度に対してサイン波で上下させる
	hoverAngle_ += hoverSpeed_;
	if (hoverAngle_ > DX_PI_F * 2.0f)
	{
		hoverAngle_ -= DX_PI_F * 2.0f;
	}

	transform_.pos.y = hoverBaseY_ + std::sinf(hoverAngle_) * hoverAmplitude_;
}