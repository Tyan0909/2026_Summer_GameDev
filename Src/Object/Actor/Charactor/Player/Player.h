#pragma once
#include <DxLib.h>
#include <vector>
#include <string>
#include "../../ActorBase.h"
#include "../../../../Object/Collider/ColliderModel.h"

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

	// 座標の衝突判定の初期座標
	static constexpr VECTOR INIT_POS = { 0.0f, 400.0f, 0.0f };




};

