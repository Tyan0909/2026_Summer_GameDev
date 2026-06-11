#include <DxLib.h>
#include "Stage.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Object/Collider/ColliderModel.h"
#include "../../../Utility/AsoUtility.h"

Stage::Stage()
	:
	ActorBase(),
	farModelId_(-1),
	modelId_(-1),
	pos_(VGet(0.0f, 0.0f, 0.0f)),
	angle_(VGet(0.0f, 0.0f, 0.0f)),
	scale_(VGet(1.0f, 1.0f, 1.0f)),
	lightHandle_(-1),
	lightHandle2_(-1),
	lightHandle3_(-1),
	lightHandle4_(-1),
	lightHandle5_(-1),
	opacityRate_(1.0f)
{
}

Stage::~Stage()
{
}

void Stage::Update(void)
{
	ApplyFarModelTransform();
}

void Stage::Draw(void)
{
	SetUseLighting(FALSE);

	SetFogEnable(TRUE);
	SetFogMode(DX_FOGMODE_LINEAR);
	// 背景（フォグ）色を暗めの紺系にして水色っぽさを抑える
	// 遠景が水色に見えるのを抑えつつ、完全な黒ではなく深い青で落ち着かせる
	SetFogColor(12, 18, 36);
	// フォグ開始を手前にして遠景を暗く見せる
	SetFogStartEnd(FOG_START_DISTANCE * 0.45f, FOG_END_DISTANCE * 0.85f);

	const VECTOR cameraPos = GetCameraPosition();
	const float cameraDistance = VSize(VSub(cameraPos, transform_.pos));

	if (transform_.modelId != -1)
	{
		MV1SetOpacityRate(transform_.modelId, opacityRate_);
	}
	if (farModelId_ != -1)
	{
		MV1SetOpacityRate(farModelId_, opacityRate_);
	}

	const bool isTransparent = opacityRate_ < 0.999f;
	if (isTransparent)
	{
		SetWriteZBuffer3D(FALSE);
	}

	if (cameraDistance <= LOD_SWITCH_DISTANCE || farModelId_ == -1)
	{
		if (transform_.modelId != -1)
		{
			MV1DrawModel(transform_.modelId);
		}
	}
	else
	{
		MV1DrawModel(farModelId_);
	}

	if (isTransparent)
	{
		SetWriteZBuffer3D(TRUE);
	}

#ifdef _DEBUG

	for (const auto& own : ownColliders_)
	{
		own.second->Draw();
	}

#endif // _DEBUG

	SetFogEnable(FALSE);
	SetUseLighting(TRUE);
}

void Stage::SetOpacityRate(float opacityRate)
{
	if (opacityRate < 0.0f)
	{
		opacityRate_ = 0.0f;
		return;
	}
	if (opacityRate > 1.0f)
	{
		opacityRate_ = 1.0f;
		return;
	}

	opacityRate_ = opacityRate;
}

bool Stage::IsAtGoal(const VECTOR& pos) const
{
	VECTOR diff = VSub(pos, GOAL_POS);
	diff.y = 0.0f;
	return VSize(diff) <= GOAL_RADIUS;
}

void Stage::DrawGoalMarker(void) const
{
	const VECTOR spherePos = VAdd(GOAL_POS, VGet(0.0f, 45.0f, 0.0f));
	const VECTOR poleTop = VAdd(GOAL_POS, VGet(0.0f, 180.0f, 0.0f));
	const int ringColor = GetColor(0, 255, 120);

	DrawSphere3D(spherePos, GOAL_RADIUS, 16, ringColor, ringColor, FALSE);
	DrawLine3D(GOAL_POS, poleTop, ringColor);
}

bool Stage::HasLineOfSight(const VECTOR& from, const VECTOR& to, float epsilon) const
{
	const auto* modelCollider = GetModelCollider();
	if (modelCollider == nullptr)
	{
		return true;
	}

	auto hit = modelCollider->GetNearestHitPolyLine(from, to, true);
	if (!hit.HitFlag)
	{
		return true;
	}

	const float hitDistance = VSize(VSub(hit.HitPosition, from));
	const float targetDistance = VSize(VSub(to, from));

	return hitDistance >= targetDistance - epsilon;
}

void Stage::UpdateOpacityForSegment(
	const VECTOR& focusPos,
	const VECTOR& cameraPos,
	float occludedOpacity,
	float epsilon)
{
	if (HasLineOfSight(focusPos, cameraPos, epsilon))
	{
		SetOpacityRate(1.0f);
	}
	else
	{
		SetOpacityRate(occludedOpacity);
	}
}

void Stage::InitLoad(void)
{
	transform_.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::MAIN_STAGE));

	farModelId_ =
		resMng_.LoadModelDuplicate(ResourceManager::SRC::MAIN_STAGE_FAR);

	// 環境光をさらに下げてステージ全体を暗めにする（黒に近い印象）
	SetGlobalAmbientLight(GetColorF(0.04f, 0.04f, 0.04f, 1.0f));
	SetLightEnable(FALSE);

	lightHandle_ = CreateDirLightHandle(VGet(0.0f, 1.0f, 0.0f));
	lightHandle2_ = CreateDirLightHandle(VGet(1.0f, 0.0f, 0.0f));
	lightHandle3_ = CreateDirLightHandle(VGet(-1.0f, 0.0f, 0.0f));
	lightHandle4_ = CreateDirLightHandle(VGet(-1.0f, 1.0f, 0.0f));
	lightHandle5_ = CreateDirLightHandle(VGet(1.0f, 1.0f, 0.0f));

	// ディフューズを若干抑えて明るさを落とす
	const float localDiffuseScale = 0.35f;
	const COLOR_F diffuse = GetColorF(DIFFUSE_STRENGTH * localDiffuseScale, DIFFUSE_STRENGTH * localDiffuseScale, DIFFUSE_STRENGTH * localDiffuseScale, 1.0f);

	SetLightEnableHandle(lightHandle_, TRUE);
	SetLightEnableHandle(lightHandle2_, TRUE);
	SetLightEnableHandle(lightHandle3_, TRUE);
	SetLightEnableHandle(lightHandle4_, TRUE);
	SetLightEnableHandle(lightHandle5_, TRUE);

	SetLightDifColorHandle(lightHandle_, diffuse);
	SetLightDifColorHandle(lightHandle2_, diffuse);
	SetLightDifColorHandle(lightHandle3_, diffuse);
	SetLightDifColorHandle(lightHandle4_, diffuse);
	SetLightDifColorHandle(lightHandle5_, diffuse);
}

void Stage::InitTransform(void)
{
	transform_.scl = { 0.5f, 0.5f, 0.5f };
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Identity();
	transform_.pos = INIT_POS;
	transform_.Update();

	ApplyFarModelTransform();
}

void Stage::InitCollider(void)
{
	MV1SetupCollInfo(transform_.modelId);

	ColliderModel* colModel =
		new ColliderModel(ColliderBase::TAG::STAGE, &transform_);

	for (const std::string& name : EXCLUDE_FRAMES)
	{
		colModel->AddExcludeFrameIds(name);
	}

	for (const std::string& name : TARGET_FRAMES)
	{
		colModel->AddTargetFrameIds(name);
	}

	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::MODEL), colModel);
}

void Stage::InitAnimation(void)
{
}

void Stage::InitPost(void)
{
}

void Stage::ApplyFarModelTransform(void)
{
	if (farModelId_ == -1)
	{
		return;
	}

	MV1SetScale(farModelId_, transform_.scl);
	MV1SetRotationXYZ(farModelId_, transform_.rot);
	MV1SetPosition(farModelId_, transform_.pos);
}

const ColliderModel* Stage::GetModelCollider(void) const
{
	const ColliderBase* stageColliderBase =
		GetOwnCollider(static_cast<int>(COLLIDER_TYPE::MODEL));

	if (stageColliderBase == nullptr ||
		stageColliderBase->GetShape() != ColliderBase::SHAPE::MODEL)
	{
		return nullptr;
	}

	return static_cast<const ColliderModel*>(stageColliderBase);
}