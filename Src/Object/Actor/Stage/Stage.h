#pragma once
//#include "../Scene/GameScene.h"
#include <DxLib.h>
#include <vector>
#include <string>
#include "../ActorBase.h"
#include "../../../Object/Collider/ColliderModel.h"

class Stage : public ActorBase
{
public:


	// 定数
	const static float GRAVITY;		// 重力
	
	// 衝突判定種別
	enum class COLLIDER_TYPE
	{
		MODEL,		// モデル
		MAX,
	};


	// コンストラクタ
	Stage(void);

	// デストラクタ
	~Stage(void);

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
	// ステージモデルの拡散光の強さ
	static constexpr float DIFFUSE_STRENGTH = 0.8f;

	// 座標の初期座標
	static constexpr VECTOR INIT_POS = { 0.0f, 0.0f, 0.0f };

	// 除外フレーム
	const std::vector<std::string> EXCLUDE_FRAMES =
	{
		"Mush",
		"Grass"
	};

	// 対象フレーム
	const std::vector<std::string> TARGET_FRAMES =
	{
		"Ground"
	};

	// ステージモデルID
	int modelId_;
	
	// 位置・角度・拡縮
	VECTOR pos_;
	VECTOR angle_;
	VECTOR scale_;

	// ライトハンドル
	int lightHandle_;		// 上から下
	int lightHandle2_;		// 右から左
	int lightHandle3_;		// 左から右
	int lightHandle4_;		// 右上から中央
	int lightHandle5_;		// 左上から中央

};