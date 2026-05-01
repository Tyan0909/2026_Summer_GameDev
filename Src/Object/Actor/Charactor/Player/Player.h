#pragma once
#include <DxLib.h>
#include <vector>
#include <string>
#include "../../ActorBase.h"
#include "../../../../Object/Collider/ColliderModel.h"

class ColliderBase;

class ResourceManager;

class Player : public ActorBase
{
	
public:

	// 定数
	static constexpr float GRAVITY = 0.5f;		// 重力

	// 衝突判定種別
	enum class COLLIDER_TYPE
	{
		MODEL,		// モデル
		MAX,
	};

	// コンストラクタ
	Player(void);

	// デストラクタ
	~Player(void);

	// 更新
	void Update(void) override;
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
	static constexpr VECTOR INIT_POS = { 0.0f, 400.0f, 0.0f };

	// 落下速度
	float gravityVelocity_;

	// 重力適用
	void ApplyGravity(void);

	// 地面との当たり判定
	bool CheckGround(VECTOR& hitPos) const;

	// 壁との当たり判定
	void ResolveWallCollision(const VECTOR& prevPos);
	bool CheckWallSegment(const VECTOR& start, const VECTOR& end, VECTOR& hitPos) const;


};

