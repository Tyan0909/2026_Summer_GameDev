#include "Player.h"
#include "../../../../Manager/Camera.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Object/Common/AnimationController.h"
#include "../../../../Object/Collider/ColliderModel.h"
#include "../../../../Object/Collider/ColliderCapsule.h"
#include "../../../../Object/Collider/ColliderLine.h"
#include "../../../../Utility/AsoUtility.h"
#include "../../../../Application.h"

Player::Player(void)
	:
	ActorBase(),
	gravityVelocity_(0.0f),
	isInputEnabled_(true),
	cameraAngles_(VGet(0.0f, 0.0f, 0.0f)),
	state_(STATE::IDLE)
{
	// アニメーションコントローラは初期化時に生成する
	animController_ = nullptr;
}

Player::~Player(void)
{
	// 動的確保したアニメーションコントローラを解放する
	delete animController_;
}

void Player::Init(void)
{
	// モデルを読み込む
	InitLoad();

	// モデルの読み込みに成功している場合のみアニメーションコントローラを生成する
	if (transform_.modelId != -1)
	{
		animController_ = new AnimationController(transform_.modelId);
	}

	// Transform の初期化を行う
	InitTransform();
	// 当たり判定の初期化を行う
	InitCollider();
	// 使用するアニメーションを登録する
	InitAnimation();
	// 必要であれば追加初期化を行う
	InitPost();

	// 初期ステートを待機に設定し、対応する開始処理を実行する
	ChangeState(STATE::IDLE, true);
}

void Player::Update(void)
{
	// カメラ操作入力を反映する
	UpdateCameraInput();
	// 移動入力を反映する
	UpdateMoveInput();

	// 移動後に壁へめり込んだ場合の補正を行う
	ResolveWallCollision();
	// 重力を適用して落下・接地処理を行う
	ApplyGravity();
	// 重力適用後に再度壁との干渉を補正する
	ResolveWallCollision();

	// 現在の状況からステート遷移を判定する
	UpdateState();

	// 現在ステートに応じた更新処理を実行する
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

	// 最終的な Transform をモデルへ反映する
	transform_.Update();

	// アニメーションが存在する場合は再生更新する
	if (animController_ != nullptr)
	{
		animController_->Update();
	}
}

void Player::UpdateMoveInput(void)
{
	VECTOR inputDir = AsoUtility::VECTOR_ZERO;

	// 前進入力を加算する
	if (isInputEnabled_ && CheckHitKey(KEY_INPUT_W))
	{
		inputDir.z += MOVE_SPEED;
	}
	// 後退入力を加算する
	if (isInputEnabled_ && CheckHitKey(KEY_INPUT_S))
	{
		inputDir.z -= MOVE_SPEED;
	}
	// 左移動入力を加算する
	if (isInputEnabled_ && CheckHitKey(KEY_INPUT_A))
	{
		inputDir.x -= MOVE_SPEED;
	}
	// 右移動入力を加算する
	if (isInputEnabled_ && CheckHitKey(KEY_INPUT_D))
	{
		inputDir.x += MOVE_SPEED;
	}

	// 入力がある場合のみ移動方向と向きを更新する
	if (!AsoUtility::EqualsVZero(inputDir))
	{
		const float moveSpeed = 1.0f;

		// 入力方向を正規化して移動方向ベクトルを作る
		VECTOR moveDir = AsoUtility::VNormalize(inputDir);
		// カメラのY回転に合わせて移動方向を回転させる
		moveDir = VTransform(moveDir, MGetRotY(cameraAngles_.y));
		// 地上移動なのでY成分は無視する
		moveDir.y = 0.0f;

		if (!AsoUtility::EqualsVZero(moveDir))
		{
			// 移動方向に向くための目標回転を求める
			const Quaternion targetRot = Quaternion::LookRotation(moveDir);

			// フレーム時間を考慮して補間率を計算する
			float turnT = SceneManager::GetInstance().GetDeltaTime() * TURN_SPEED;
			if (turnT < 0.0f)
			{
				turnT = 0.0f;
			}
			if (turnT > 1.0f)
			{
				turnT = 1.0f;
			}

			// 現在の向きから目標方向へ滑らかに回転させる
			transform_.quaRot = Quaternion::Slerp(transform_.quaRot, targetRot, turnT);
		}

		// プレイヤー位置を移動方向へ加算する
		transform_.pos = VAdd(transform_.pos, VScale(moveDir, moveSpeed));
	}
}

void Player::UpdateCameraInput(void)
{
	// 入力が無効ならカメラ操作も行わない
	if (!isInputEnabled_)
	{
		return;
	}

	// 上キーでカメラを上向きに回転する
	if (CheckHitKey(KEY_INPUT_UP))
	{
		cameraAngles_.x -= CAMERA_ROT_SPEED;
	}
	// 下キーでカメラを下向きに回転する
	if (CheckHitKey(KEY_INPUT_DOWN))
	{
		cameraAngles_.x += CAMERA_ROT_SPEED;
	}
	// 左キーでカメラを左回転する
	if (CheckHitKey(KEY_INPUT_LEFT))
	{
		cameraAngles_.y -= CAMERA_ROT_SPEED;
	}
	// 右キーでカメラを右回転する
	if (CheckHitKey(KEY_INPUT_RIGHT))
	{
		cameraAngles_.y += CAMERA_ROT_SPEED;
	}

	// ピッチ角が下限を超えないように制限する
	if (cameraAngles_.x < CAMERA_PITCH_MIN)
	{
		cameraAngles_.x = CAMERA_PITCH_MIN;
	}
	// ピッチ角が上限を超えないように制限する
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
	// 外部から指定されたカメラ角度をそのまま反映する
	cameraAngles_ = angles;
}

VECTOR Player::GetCameraWorldPos(void) const
{
	// プレイヤー基準の三人称カメラ位置を取得する
	VECTOR cameraOffset = TPS_CAMERA_LOCAL_POS;
	// プレイヤーではなくカメラY角に応じてオフセットを回転させる
	cameraOffset = VTransform(cameraOffset, MGetRotY(cameraAngles_.y));
	// プレイヤー座標に加算してワールド座標へ変換する
	return VAdd(transform_.pos, cameraOffset);

	// 一人称視点に戻す場合
	// VECTOR cameraOffset = FPS_CAMERA_LOCAL_POS;
	// cameraOffset = VTransform(cameraOffset, MGetRotY(cameraAngles_.y));
	// return VAdd(transform_.pos, cameraOffset);
}

VECTOR Player::GetCameraForward(void) const
{
	const float pitch = cameraAngles_.x;
	const float yaw = cameraAngles_.y;

	// ピッチ角とヨー角から前方ベクトルを作成する
	VECTOR forward = VGet(
		sinf(yaw) * cosf(pitch),
		-sinf(pitch),
		cosf(yaw) * cosf(pitch));

	const float length = VSize(forward);
	// ベクトル長が極端に小さい場合はデフォルト前方を返す
	if (length <= 0.0001f)
	{
		return VGet(0.0f, 0.0f, 1.0f);
	}

	// 正規化した前方ベクトルを返す
	return VScale(forward, 1.0f / length);
}

void Player::SetPos(const VECTOR& pos)
{
	// 座標を直接設定し、Transform を更新する
	transform_.pos = pos;
	transform_.Update();
}

void Player::SetInputEnabled(bool isEnabled)
{
	// 入力の有効・無効を切り替える
	isInputEnabled_ = isEnabled;
}

void Player::InitLoad(void)
{
	// プレイヤーモデルを読み込んで Transform に設定する
	transform_.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::PLAYER));

	// 読み込み失敗時は以降のモデル依存処理を避ける
	if (transform_.modelId == -1)
	{
		return;
	}
}

void Player::InitTransform(void)
{
	// モデルの拡大率を設定する
	transform_.scl = { 0.5f,0.5f,0.5f };
	// ワールド回転を単位クォータニオンで初期化する
	transform_.quaRot = Quaternion::Identity();

	// モデル自体の向き補正をローカル回転に設定する
	transform_.quaRotLocal = Quaternion::AngleAxis(DX_PI_F, VGet(0.0f, 1.0f, 0.0f));

	// 初期座標を設定する
	transform_.pos = INIT_POS;
	// 初期 Transform を反映する
	transform_.Update();
}

void Player::InitCollider(void)
{
	// 地面チェックなどに使うラインコライダーを生成する
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::LINE), colLine);

	// プレイヤー本体用のカプセルコライダーを生成する
	ColliderCapsule* colCapsule = new ColliderCapsule(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_CAPSULE_TOP_LOCAL_POS, COL_CAPSULE_DOWN_LOCAL_POS,
		COL_CAPSULE_RADIUS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);
}

void Player::InitAnimation(void)
{
	// アニメーションコントローラ未生成時は何もしない
	if (animController_ == nullptr)
	{
		return;
	}

	std::string path = Application::PATH_MODEL + "Player/Animation/";

	// 待機アニメーションを登録する
	animController_->Add((int)ANIM_TYPE::IDLE, path + "Idle.mv1", 20.0f);
	// しゃがみアニメーションを登録する
	animController_->Add((int)ANIM_TYPE::CROUCHED, path + "Crouched.mv1", 20.0f);
	// 歩きアニメーションを登録する
	animController_->Add((int)ANIM_TYPE::WALK, path + "Walking.mv1", 60.0f);
	// 走りアニメーションは未登録
	// animController_->Add((int)ANIM_TYPE::RUN, path + "Run.mv1", 20.0f);
}

void Player::InitPost(void)
{
}

void Player::ApplyGravity(void)
{
	VECTOR hitPos = AsoUtility::VECTOR_ZERO;

	// 接地中かつ下向き速度以下なら地面位置へ補正して終了する
	if (CheckGround(hitPos) && gravityVelocity_ <= 0.0f)
	{
		transform_.pos.y = hitPos.y + GROUND_OFFSET;
		gravityVelocity_ = 0.0f;
		return;
	}

	// 重力加速度を適用して落下速度を更新する
	gravityVelocity_ -= GRAVITY;
	// 終端速度を超えないように制限する
	if (gravityVelocity_ < GRAVITY_TERMINAL)
	{
		gravityVelocity_ = GRAVITY_TERMINAL;
	}

	// Y座標へ落下速度を反映する
	transform_.pos.y += gravityVelocity_;

	// 落下後に接地した場合は地面位置へ補正する
	if (CheckGround(hitPos) && gravityVelocity_ <= 0.0f)
	{
		transform_.pos.y = hitPos.y + GROUND_OFFSET;
		gravityVelocity_ = 0.0f;
	}
}

bool Player::CheckGround(VECTOR& hitPos) const
{
	// プレイヤーの少し上から下方向へ地面判定ラインを飛ばす
	const VECTOR start = VAdd(transform_.pos, VGet(0.0f, 10.0f, 0.0f));
	const VECTOR end = VAdd(transform_.pos, VGet(0.0f, -GROUND_CHECK_DISTANCE, 0.0f));

	for (const auto& hitCollider : hitColliders_)
	{
		// モデルコライダー以外は地面判定対象にしない
		if (hitCollider == nullptr ||
			hitCollider->GetShape() != ColliderBase::SHAPE::MODEL)
		{
			continue;
		}

		const auto* modelCollider = static_cast<const ColliderModel*>(hitCollider);
		auto hit = modelCollider->GetNearestHitPolyLine(start, end, true);
		if (hit.HitFlag)
		{
			// ヒット地点を保持し、少し上に補正して返す
			hitPos = hit.HitPosition;
			hitPos.y += 5.0f;
			return true;
		}
	}

	// 地面にヒットしなかった
	return false;
}

void Player::ResolveWallCollision(void)
{
	// 自身のカプセルコライダーを取得する
	const auto* capsule = static_cast<const ColliderCapsule*>(
		GetOwnCollider(static_cast<int>(COLLIDER_TYPE::CAPSULE)));

	// カプセルが無い場合は壁補正できない
	if (capsule == nullptr)
	{
		return;
	}

	for (const auto& hitCollider : hitColliders_)
	{
		// モデルコライダー以外は壁判定対象にしない
		if (hitCollider == nullptr ||
			hitCollider->GetShape() != ColliderBase::SHAPE::MODEL)
		{
			continue;
		}

		const auto* modelCollider = static_cast<const ColliderModel*>(hitCollider);

		// 壁法線に沿ってプレイヤーを押し戻し、めり込みを解消する
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
	// 現在の状況から次のステートを求め、必要なら遷移する
	ChangeState(GetNextState());
}

Player::STATE Player::GetNextState(void) const
{
	VECTOR hitPos = AsoUtility::VECTOR_ZERO;

	// 地面に接していなければジャンプ中とみなす
	if (!CheckGround(hitPos))
	{
		return STATE::JUMP;
	}

	// 入力無効時は待機状態にする
	if (!isInputEnabled_)
	{
		return STATE::IDLE;
	}

	// しゃがみキー入力中はしゃがみ状態にする
	if (CheckHitKey(KEY_INPUT_C))
	{
		return STATE::CROUCHED;
	}

	// 移動入力がある場合は歩きまたは走りを返す
	if (HasMoveInput())
	{
		// Shift 押下中は走り状態にする
		if (CheckHitKey(KEY_INPUT_LSHIFT) || CheckHitKey(KEY_INPUT_RSHIFT))
		{
			return STATE::RUN;
		}

		// Shift が無ければ歩き状態にする
		return STATE::WALK;
	}

	// それ以外は待機状態にする
	return STATE::IDLE;
}

void Player::ChangeState(STATE newState, bool isForce)
{
	// 強制変更でなく、同じステートなら何もしない
	if (!isForce && state_ == newState)	
	{
		return;
	}

	// ステートを更新する
	state_ = newState;

	// 遷移先ステートごとの開始処理を呼ぶ
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
	// 入力無効なら移動入力なしとして扱う
	if (!isInputEnabled_)
	{
		return false;
	}

	// WASD のいずれかが押されていれば移動入力あり
	return CheckHitKey(KEY_INPUT_W) ||
		CheckHitKey(KEY_INPUT_S) ||
		CheckHitKey(KEY_INPUT_A) ||
		CheckHitKey(KEY_INPUT_D);
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
		// 待機アニメーションをループ再生する
		animController_->Play((int)ANIM_TYPE::IDLE, true);
	}
}

void Player::OnEnterWalk(void)
{
	if (animController_ != nullptr)
	{
		// 歩きアニメーションをループ再生する
		animController_->Play((int)ANIM_TYPE::WALK, true);
	}
}

void Player::OnEnterRun(void)
{
	if (animController_ != nullptr)
	{
		// 走りアニメ未登録のため暫定で歩きアニメーションを再生する
		animController_->Play((int)ANIM_TYPE::WALK, true);
	}
}

void Player::OnEnterJump(void)
{
	if (animController_ != nullptr)
	{
		// ジャンプアニメ未登録のため暫定で待機アニメーションを再生する
		animController_->Play((int)ANIM_TYPE::IDLE, true);
	}
}

void Player::OnEnterCrouched(void)
{
	if (animController_ != nullptr)
	{
		// しゃがみアニメーションをループ再生する
		animController_->Play((int)ANIM_TYPE::CROUCHED, true);
	}
}

