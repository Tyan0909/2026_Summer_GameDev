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
	lightHandle5_(-1)
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
	SetFogColor(160, 180, 200);
	SetFogStartEnd(FOG_START_DISTANCE, FOG_END_DISTANCE);

	const VECTOR cameraPos = GetCameraPosition();
	const float cameraDistance = VSize(VSub(cameraPos, transform_.pos));

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

#ifdef _DEBUG

	for (const auto& own : ownColliders_)
	{
		own.second->Draw();
	}

#endif // _DEBUG

	SetFogEnable(FALSE);
	SetUseLighting(TRUE);
}

void Stage::InitLoad(void)
{
	transform_.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::MAIN_STAGE));

	farModelId_ =
		resMng_.LoadModelDuplicate(ResourceManager::SRC::MAIN_STAGE_FAR);

	// グローバルアンビエントライトの設定
	SetGlobalAmbientLight(GetColorF(0.3f, 0.3f, 0.3f, 1.0f));

	// デフォルトライトの影響を無効化
	SetLightEnable(FALSE);

	// ディレクショナルライトの作成
	lightHandle_ = CreateDirLightHandle(VGet(0.0f, 1.0f, 0.0f));
	lightHandle2_ = CreateDirLightHandle(VGet(1.0f, 0.0f, 0.0f));
	lightHandle3_ = CreateDirLightHandle(VGet(-1.0f, 0.0f, 0.0f));
	lightHandle4_ = CreateDirLightHandle(VGet(-1.0f, 1.0f, 0.0f));
	lightHandle5_ = CreateDirLightHandle(VGet(1.0f, 1.0f, 0.0f));

	const COLOR_F diffuse = GetColorF(DIFFUSE_STRENGTH, DIFFUSE_STRENGTH, DIFFUSE_STRENGTH, 1.0f);

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
	// DxLib側の衝突判定セットアップ
	MV1SetupCollInfo(transform_.modelId);

	// モデルコライダー
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
	// アニメーションの初期化は必要に応じて実装
}

void Stage::InitPost(void)
{
	// ステージの初期化後の個別処理は必要に応じて実装
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
