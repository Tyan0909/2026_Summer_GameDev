#include <string>
#include "../../../../Application.h"
#include "../../../../Utility/AsoUtility.h"
#include "../../../../Manager/InputManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/Camera.h"
#include "../../../Common/AnimationController.h"
#include "../../../Common/Capsule.h"
#include "../../../Common/Collider.h"
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
	auto& ins = InputManager::GetInstance();

	// 移動量をリセット
	movePow_ = VGet(0.0f, 0.0f, 0.0f);

	// X軸回転をのぞいた、重力方向に垂直なカメラ角度を取得
	Quaternion camRot = SceneManager::GetInstance().GetCamera()->GetQuaRotOutX();

	// 回転したい角度
	double rotRad = 0;

	VECTOR dir = VGet(0.0f, 0.0f, 0.0f);

	// カメラ方向に前進
	if (ins.IsNew(KEY_INPUT_W) || ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::TOP))
	{
		rotRad = AsoUtility::Deg2RadD(0.0);
		dir = camRot.GetForward();
	}

	// カメラ方向に後退
	if (ins.IsNew(KEY_INPUT_S) || ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
	{
		rotRad = AsoUtility::Deg2RadD(180.0);
		dir = camRot.GetBack();
	}

	// カメラ方向に右移動
	if (ins.IsNew(KEY_INPUT_D) || ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::RIGHT))
	{
		rotRad = AsoUtility::Deg2RadD(90.0);
		dir = camRot.GetRight();
	}

	// カメラ方向に左移動
	if (ins.IsNew(KEY_INPUT_A) || ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::LEFT))
	{
		rotRad = AsoUtility::Deg2RadD(-90.0);
		dir = camRot.GetLeft();
	}

	if (!AsoUtility::EqualsVZero(dir) && (isJump_ || IsEndlanding())) {

		// 移動処理
		speed_ = SPEED_MOVE;

		// ダッシュ処理 必要であれば実装
		/*if (ins.IsNew(KEY_INPUT_RSHIFT))
		{
			speed_ = SPEED_RUN;
		}*/
		moveDir_ = dir;
		movePow_ = VScale(dir, speed_);

		// 回転処理
		SetGoalRotate(rotRad);

		if (!isJump_ && IsEndlanding())
		{
			// アニメーション
			if (ins.IsNew(KEY_INPUT_RSHIFT))
			{
				//animationController_->Play((int)ANIM_TYPE::FAST_RUN);
			}
			else
			{
				//animationController_->Play((int)ANIM_TYPE::RUN);
			}
		}

	}
	else
	{
		if (!isJump_ && IsEndlanding())
		{
			//animationController_->Play((int)ANIM_TYPE::IDLE);
		}
	}


}

void Player::ProcessJump(void)
{

	// ジャンプ処理が必要なければ処理しない

	//bool isHit = CheckHitKey(KEY_INPUT_BACKSLASH);

	//// ジャンプ
	//if (isHit && (isJump_ || IsEndLanding()))
	//{

	//	if (!isJump_)
	//	{
	//		// 制御無しジャンプ
	//		//mAnimationController->Play((int)ANIM_TYPE::JUMP);
	//		// ループしないジャンプ
	//		//mAnimationController->Play((int)ANIM_TYPE::JUMP, false);
	//		// 切り取りアニメーション
	//		//mAnimationController->Play((int)ANIM_TYPE::JUMP, false, 13.0f, 24.0f);
	//		// 無理やりアニメーション
	//		animationController_->Play((int)ANIM_TYPE::JUMP, true, 13.0f, 25.0f);
	//		animationController_->SetEndLoop(23.0f, 25.0f, 5.0f);
	//	}

	//	isJump_ = true;

	//	// ジャンプの入力受付時間をヘラス
	//	stepJump_ += scnMng_.GetDeltaTime();
	//	if (stepJump_ < TIME_JUMP_IN)
	//	{
	//		jumpPow_ = VScale(AsoUtility::DIR_U, POW_JUMP);
	//	}

	//}

	//// ボタンを離したらジャンプ力に加算しない
	//if (!isHit)
	//{
	//	stepJump_ = TIME_JUMP_IN;
	//}

}

void Player::SetGoalRotate(double rotRad)
{

	VECTOR cameraRot = SceneManager::GetInstance().GetCamera()->GetAngles();
	Quaternion axis = Quaternion::AngleAxis((double)cameraRot.y + rotRad, AsoUtility::AXIS_Y);

	// 現在設定されている回転との角度差を取る
	double angleDiff = Quaternion::Angle(axis, goalQuaRot_);

	// しきい値
	if (angleDiff > 0.1)
	{
		stepRotTime_ = TIME_ROT;
	}

	goalQuaRot_ = axis;

}

void Player::Rotate(void)
{

	stepRotTime_ -= scnMng_.GetDeltaTime();

	// 回転の球面補間
	playerRotY_ = Quaternion::Slerp(
		playerRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);

}

void Player::Collision(void)
{

	// 現在座標を起点に移動後座標を決める
	movedPos_ = VAdd(transform_.pos, movePow_);

	// 衝突(カプセル)
	CollisionCapsule();

	// 衝突(重力)
	CollisionGravity();

	// 移動
	transform_.pos = movedPos_;

}

void Player::CollisionGravity(void)
{

	// ジャンプ量を加算
	movedPos_ = VAdd(movedPos_, jumpPow_);

	// 重力方向
	VECTOR dirGravity = AsoUtility::DIR_D;

	// 重力方向の反対
	VECTOR dirUpGravity = AsoUtility::DIR_U;

	// 重力の強さ
	float gravityPow = 9.8f;

	float checkPow = 10.0f;
	gravHitPosUp_ = VAdd(movedPos_, VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * 2.0f));
	gravHitPosDown_ = VAdd(movedPos_, VScale(dirGravity, checkPow));
	for (const auto c : colliders_)
	{

		// 地面との衝突
		auto hit = MV1CollCheck_Line(
			c->modelId_, -1, gravHitPosUp_, gravHitPosDown_);

		// 最初は上の行のように実装して、木の上に登ってしまうことを確認する
		//if (hit.HitFlag > 0)
		if (hit.HitFlag > 0 && VDot(dirGravity, jumpPow_) > 0.9f)
		{

			// 衝突地点から、少し上に移動
			movedPos_ = VAdd(hit.HitPosition, VScale(dirUpGravity, 2.0f));

			// ジャンプリセット
			jumpPow_ = AsoUtility::VECTOR_ZERO;
			stepJump_ = 0.0f;

			if (isJump_)
			{
				// 着地モーション
				/*animationController_->Play(
					(int)ANIM_TYPE::JUMP, false, 29.0f, 45.0f, false, true);*/
			}

			isJump_ = false;

		}

	}

}

void Player::CollisionCapsule(void)
{

	// カプセルを移動させる
	Transform trans = Transform(transform_);
	trans.pos = movedPos_;
	trans.Update();
	Capsule cap = Capsule(*capsule_, trans);

	// カプセルとの衝突判定
	for (const auto c : colliders_)
	{

		auto hits = MV1CollCheck_Capsule(
			c->modelId_, -1,
			cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius());

		for (int i = 0; i < hits.HitNum; i++)
		{

			auto hit = hits.Dim[i];

			for (int tryCnt = 0; tryCnt < 10; tryCnt++)
			{

				int pHit = HitCheck_Capsule_Triangle(
					cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius(),
					hit.Position[0], hit.Position[1], hit.Position[2]);

				if (pHit)
				{
					movedPos_ = VAdd(movedPos_, VScale(hit.Normal, 1.0f));
					// カプセルを移動させる
					trans.pos = movedPos_;
					trans.Update();
					continue;
				}

				break;

			}

		}

		// 検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(hits);

	}

}

void Player::CalcGravityPow(void)
{

	// 重力方向
	VECTOR dirGravity = AsoUtility::DIR_D;

	// 重力の強さ
	float gravityPow = 9.8f;

	// 重力
	VECTOR gravity = VScale(dirGravity, gravityPow);
	jumpPow_ = VAdd(jumpPow_, gravity);

	// 最初は実装しない。地面と突き抜けることを確認する。
	// 内積
	float dot = VDot(dirGravity, jumpPow_);
	if (dot >= 0.0f)
	{
		// 重力方向と反対方向(マイナス)でなければ、ジャンプ力を無くす
		jumpPow_ = gravity;
	}

}

bool Player::IsEndlanding(void)
{

	bool ret = true;

	// アニメーションがジャンプではない
	/*if (animationController_->GetPlayType() != (int)ANIM_TYPE::JUMP)
	{
		return ret;
	}*/

	// アニメーションが終了しているか
	/*if (animationController_->IsEnd())
	{
		return ret;
	}*/

	return false;

}


