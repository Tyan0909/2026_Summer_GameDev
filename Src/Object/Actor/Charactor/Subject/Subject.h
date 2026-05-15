#pragma once
#include <DxLib.h>
#include <vector>
#include <string>
#include "../../ActorBase.h"
#include "../../../../Object/Collider/ColliderModel.h"

class ColliderBase;
class ResourceManager;

class Subject : public ActorBase
{
public:

	// 定数
	static constexpr float GRAVITY = 0.5f;		// 重力

	// 移動速度
	static constexpr float MOVE_SPEED = 150.f;

	// 衝突判定用線分開始
	static constexpr VECTOR
		COL_LINE_START_LOCAL_POS = { 0.0f, 80.0f, 0.0f };

	// 衝突判定用線分終了
	static constexpr VECTOR
		COL_LINE_END_LOCAL_POS = { 0.0f, -10.0f, 0.0f };

	// 衝突判定用カプセル上部球体
	static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS = { 0.0f, 110.0f, 0.0f };

	// 衝突判定用カプセル下部球体
	static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS = { 0.0f, 30.0f, 0.0f };

	// 衝突判定用カプセル球体半径
	static constexpr float COL_CAPSULE_RADIUS = 20.0f;

	// 衝突判定種別
	enum class COLLIDER_TYPE
	{
		MODEL,		// モデル
		LINE,		// 線分
		CAPSULE,	// カプセル
		MAX,
	};

	// コンストラクタ
	Subject(void);

	// デストラクタ
	~Subject(void);

	// 更新
	void Update(void) override;
	void SetPos(const VECTOR& pos);
	void SetInputEnabled(bool isEnabled);

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

	// 座標の衝突判定の初期座標
	static constexpr VECTOR INIT_POS = { 0.0f, 1000.0f, 0.0f };

	// 落下速度
	float gravityVelocity_;
	bool isInoputEnabled_;

	// 重力
	/*static constexpr float GRAVITY = 0.98f;*/

	// 重力適応
	void ApplyGravity(void);

	// 地面との接地判定
	bool CheckGround(VECTOR& hitPos) const;

	// 壁との当たり判定
	void ResolveWallCollision(const VECTOR& prevPos);
	bool CheckWallSegment(const VECTOR& start, const VECTOR& end, VECTOR& hitPos) const;

};

