#pragma once
#include <DxLib.h>
#include <vector>
#include <string>
#include "../../ActorBase.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Object/Collider/ColliderModel.h"

class ColliderBase;

class Subject : public ActorBase
{
public:

	// 定数
	static constexpr float GRAVITY = 0.5f;
	static constexpr float MOVE_SPEED = 1.5f;

	// 移動範囲チェック用
	static constexpr VECTOR
		COL_LINE_START_LOCAL_POS = { 0.0f, 80.0f, 0.0f };
	static constexpr VECTOR
		COL_LINE_END_LOCAL_POS = { 0.0f, -10.0f, 0.0f };
	static constexpr VECTOR
		COL_CAPSULE_TOP_LOCAL_POS = { 0.0f, 110.0f, 0.0f };
	static constexpr VECTOR
		COL_CAPSULE_DOWN_LOCAL_POS = { 0.0f, 30.0f, 0.0f };
	static constexpr float COL_CAPSULE_RADIUS = 20.0f;

	// 衝突判定種別
	enum class COLLIDER_TYPE
	{
		MODEL,
		LINE,
		CAPSULE,
		MAX,
	};

	// 行動状態
	enum class ACTION_STATE
	{
		MOVE,
		ATTACK,
	};

	// コンストラクタ
	Subject(void);

	// デストラクタ
	~Subject(void);

	// 更新
	void Update(void) override;
	void SetPos(const VECTOR& pos);
	void SetInputEnabled(bool isEnabled);
	void SetModelSrc(ResourceManager::SRC modelSrc);
	void SetMoveArea(const VECTOR& minPos, const VECTOR& maxPos);

	bool IsInAttackRange(const VECTOR& targetPos) const;
	bool CanStartAttack(void) const;
	bool StartAttack(const VECTOR& targetPos);
	bool ConsumeAttackHit(void) ;

	// 追加: スタン（フレーム数）を付与
	void Stun(int frames);
	bool IsStunned() const { return stunFrames_ > 0; }

protected:

	// リソースロード
	void InitLoad(void) override;

	// 大きさ、回転、座標の初期化
	void InitTransform(void) override;

	// 衝突判定の初期化
	void InitCollider(void) override;

	// アニメーションの初期化
	void InitAnimation(void) override;

	// 初期化後の個別処理
	void InitPost(void) override;

private:

	// 定数
	static constexpr float GRAVITY_TERMINAL = -20.0f;
	static constexpr float GROUND_CHECK_DISTANCE = 500.0f;
	static constexpr float GROUND_OFFSET = 1.0f;
	static constexpr float WALL_CHECK_HEIGHT = 30.0f;
	static constexpr float WALL_PUSH_BACK = 2.0f;
	static constexpr int RANDOM_DIR_CHANGE_MIN = 60;
	static constexpr int RANDOM_DIR_CHANGE_MAX = 180;

	// 初期位置
	static constexpr VECTOR INIT_POS = { 0.0f, 1000.0f, 0.0f };

	// 攻撃
	static constexpr float ATTACK_RANGE = 85.0f;
	static constexpr int ATTACK_COOLDOWN_MAX = 90;
	static constexpr int ATTACK_FRAME_MAX = 24;
	static constexpr int ATTACK_HIT_FRAME = 12;
	static constexpr float ATTACK_STRETCH_MAX = 0.4f;

	// 重力速度
	float gravityVelocity_;
	bool isInoputEnabled_;
	ResourceManager::SRC modelSrc_;

	VECTOR moveAreaMin_;
	VECTOR moveAreaMax_;
	VECTOR moveDir_;
	int moveDirChangeFrame_;

	VECTOR baseScale_;
	ACTION_STATE actionState_;
	int attackCooldownFrame_;
	int attackFrame_;
	VECTOR attackTargetPos_;
	bool isAttackHitPending_;

	int stunFrames_ = 0; // 追加: スタン残りフレーム

	// 重力適用
	void ApplyGravity(void);

	// 地面との接地判定
	bool CheckGround(VECTOR& hitPos) const;

	// 壁との当たり判定
	void ResolveWallCollision(const VECTOR& prevPos);
	bool CheckWallSegment(const VECTOR& start, const VECTOR& end, VECTOR& hitPos) const;

	void UpdateRandomMove(void);
	void UpdateAttack(void);
	void PickRandomMoveDirection(void);
	void FaceMoveDirection(void);
	void FaceTarget(const VECTOR& targetPos);
	void ClampToMoveArea(void);
};

