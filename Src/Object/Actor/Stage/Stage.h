#pragma once
#include <DxLib.h>
#include <vector>
#include <string>
#include "../ActorBase.h"
#include "../../../Object/Collider/ColliderModel.h"

class Stage : public ActorBase
{
public:
	// 定数
	const static float GRAVITY;

	// 衝突判定用
	enum class COLLIDER_TYPE
	{
		MODEL,
		MAX,
	};

	// コンストラクタ
	Stage(void);

	// デストラクタ
	~Stage(void);

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

	void SetOpacityRate(float opacityRate);

	bool IsAtGoal(const VECTOR& pos) const;
	void DrawGoalMarker(void) const;
	bool HasLineOfSight(const VECTOR& from, const VECTOR& to, float epsilon = 1.0f) const;
	void UpdateOpacityForSegment(
		const VECTOR& focusPos,
		const VECTOR& cameraPos,
		float occludedOpacity,
		float epsilon);

protected:
	// リソースロード
	void InitLoad(void) override;

	// 大きさ、回転、座標の初期化
	void InitTransform(void) override;

	// 衝突判定の初期化
	void InitCollider(void) override;

	// アニメーションの初期化
	void InitAnimation(void) override;

	// 後処理の個別初期化
	void InitPost(void) override;

private:
	// 定数
	static constexpr float DIFFUSE_STRENGTH = 0.8f;
	static constexpr float LOD_SWITCH_DISTANCE = 1800.0f;
	static constexpr float FOG_START_DISTANCE = 1200.0f;
	static constexpr float FOG_END_DISTANCE = 3500.0f;

	static constexpr VECTOR INIT_POS = { 0.0f, 0.0f, 0.0f };
	static constexpr VECTOR GOAL_POS = { 520.0f, 0.0f, 520.0f };
	static constexpr float GOAL_RADIUS = 80.0f;

	// 除外フレーム
	const std::vector<std::string> EXCLUDE_FRAMES =
	{
		"Mush"
	};

	// 対象フレーム
	const std::vector<std::string> TARGET_FRAMES =
	{
		"Ground"
	};

	// 遠景用ローポリモデルID
	int farModelId_;

	int modelId_;

	VECTOR pos_;
	VECTOR angle_;
	VECTOR scale_;

	int lightHandle_;
	int lightHandle2_;
	int lightHandle3_;
	int lightHandle4_;
	int lightHandle5_;

	float opacityRate_;

	void ApplyFarModelTransform(void);
	const ColliderModel* GetModelCollider(void) const;
};