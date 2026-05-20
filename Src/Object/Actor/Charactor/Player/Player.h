#pragma once
#include <DxLib.h>
#include <vector>
#include <string>
#include "../../ActorBase.h"
#include "../../../../Object/Collider/ColliderModel.h"

class ColliderBase;
class ResourceManager;
class AnimationController;

class Player : public ActorBase
{
public:

	// 定数
	static constexpr float GRAVITY = 0.5f;
	static constexpr float MOVE_SPEED = 150.f;

	static constexpr VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 80.0f, 0.0f };
	static constexpr VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, -10.0f, 0.0f };

	static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS = { 0.0f, 110.0f, 0.0f };
	static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS = { 0.0f, 30.0f, 0.0f };
	static constexpr float COL_CAPSULE_RADIUS = 20.0f;

	// 三人称視点でのカメラ相対位置
	static constexpr VECTOR TPS_CAMERA_LOCAL_POS = { 0.0f, 120.0f, -180.0f };

	enum class COLLIDER_TYPE
	{
		MODEL,
		LINE,
		CAPSULE,
		MAX,
	};

	enum class ANIM_TYPE
	{
		IDLE,
		RUN,
		JUMP,
		CROUCHED,
		WALK,
		MAX,
	};

	enum class STATE
	{
		IDLE,
		WALK,
		RUN,
		JUMP,
		CROUCHED,
	};

	// プレイヤーの各種メンバを初期化するコンストラクタ
	Player(void);

	// 動的確保したアニメーションコントローラを解放するデストラクタ
	~Player(void);

	// プレイヤー全体の初期化を行う
	// モデル、Transform、Collider、Animation、初期ステートを設定する
	void Init(void);

	// 毎フレームの更新処理を行う
	// 入力、移動、重力、壁押し戻し、ステート更新、アニメ更新を担当する
	void Update(void) override;

	// プレイヤーの座標を直接設定する
	void SetPos(const VECTOR& pos);

	// プレイヤーの入力受付の有効/無効を切り替える
	void SetInputEnabled(bool isEnabled);

	// カメラの回転角を取得する
	const VECTOR& GetCameraAngles(void) const;

	// カメラの回転角を設定する
	void SetCameraAngles(const VECTOR& angles);

	// プレイヤー基準のカメラワールド座標を取得する
	VECTOR GetCameraWorldPos(void) const;

	// カメラが向いている前方ベクトルを取得する
	VECTOR GetCameraForward(void) const;

	// ダメージを受ける
	void TakeDamage(int damage);

	// ダメージを受けることができるかどうか
	bool CanTakeDamage(void) const;

	// プレイヤーが死亡しているか
	bool IsDead(void) const;

	// 現在のHP
	int GetHp(void) const;

	// 最大HP
	int GetHpMax(void) const;

	// HPの割合 (0.0f~1.0f)
	float GetHpRate(void) const;

protected:
	// プレイヤーモデルの読み込みを行う
	void InitLoad(void) override;

	// 拡大率、回転、初期座標など Transform の初期化を行う
	void InitTransform(void) override;

	// プレイヤー用のコライダーを生成して登録する
	void InitCollider(void) override;

	// プレイヤー用アニメーションを読み込んで登録する
	void InitAnimation(void) override;

	// 追加の初期配置処理を行う
	void InitPost(void) override;

private:
	static constexpr float GRAVITY_TERMINAL = -20.0f;
	static constexpr float GROUND_CHECK_DISTANCE = 500.0f;
	static constexpr float GROUND_OFFSET = 1.0f;
	static constexpr float WALL_CHECK_HEIGHT = 30.0f;
	static constexpr float WALL_PUSH_BACK = 2.0f;
	static constexpr float WALL_NORMAL_Y_MAX = 0.4f;

	static constexpr float CAMERA_ROT_SPEED = DX_PI_F / 180.0f;
	static constexpr float CAMERA_PITCH_MIN = -DX_PI_F * 0.45f;
	static constexpr float CAMERA_PITCH_MAX = DX_PI_F * 0.45f;
	static constexpr float TURN_SPEED = 10.0f;

	static constexpr VECTOR INIT_POS = { 300.0f, 100.0f, 100.0f };

	static constexpr int HP_MAX = 3;
	static constexpr int DAMAGE_COOLDOWN_MAX = 60;

	float gravityVelocity_;
	bool isInputEnabled_;
	VECTOR cameraAngles_;

	int hp_;
	int damageCooldownFrame_;

	AnimationController* animController_;

	// WASD入力に応じて移動方向と向きを更新する
	void UpdateMoveInput(void);

	// 矢印キー入力に応じてカメラ角度を更新する
	void UpdateCameraInput(void);

	// 重力を適用し、地面に接地していれば位置補正を行う
	void ApplyGravity(void);

	// 地面との当たり判定を行い、接地位置を取得する
	bool CheckGround(VECTOR& hitPos) const;

	// 壁との衝突を解消するために押し戻し処理を行う
	void ResolveWallCollision(void);

	// 次のステート判定とステート変更をまとめて行う
	void UpdateState(void);

	// 現在の入力や接地状況から次に遷移すべきステートを返す
	STATE GetNextState(void) const;

	// ステートを変更し、遷移時の初期処理を実行する
	void ChangeState(STATE newState, bool isForce = false);

	// 移動入力があるかどうかを判定する
	bool HasMoveInput(void) const;

	// 待機ステート中の更新処理
	void UpdateIdle(void);

	// 歩きステート中の更新処理
	void UpdateWalk(void);

	// 走りステート中の更新処理
	void UpdateRun(void);

	// ジャンプステート中の更新処理
	void UpdateJump(void);

	// しゃがみステート中の更新処理
	void UpdateCrouched(void);

	// 待機ステートへ入ったときの初期処理
	// 主に待機アニメーション再生を行う
	void OnEnterIdle(void);

	// 歩きステートへ入ったときの初期処理
	// 主に歩きアニメーション再生を行う
	void OnEnterWalk(void);

	// 走りステートへ入ったときの初期処理
	// 主に走り用アニメーション再生を行う
	void OnEnterRun(void);

	// ジャンプステートへ入ったときの初期処理
	// 主にジャンプ開始時のアニメーション設定を行う
	void OnEnterJump(void);

	// しゃがみステートへ入ったときの初期処理
	// 主にしゃがみアニメーション再生を行う
	void OnEnterCrouched(void);

	STATE state_;
};

