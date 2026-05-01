#include <string>
#include "../../../../Application.h"
#include "../../../../Utility/AsoUtility.h"
#include "../../../../Manager/InputManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/Camera.h"
#include "../../../Common/AnimationController.h"
#include "../../../Common/Capsule.h"
#include "Player.h"

Player::Player(void)
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
	// 状態遷移
	switch (state_)
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

