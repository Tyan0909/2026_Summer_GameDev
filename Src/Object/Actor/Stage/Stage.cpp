#include <DxLib.h>
#include "Stage.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Object/Collider/ColliderModel.h"
#include "../../../Utility/AsoUtility.h"

Stage::Stage()
	:
	ActorBase()
{
}

Stage::~Stage()
{
}

void Stage::Update(void)
{
}

void Stage::InitLoad(void)
{
	transform_.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::MAIN_STAGE));

	// グローバルアンビエントライトの設定
	SetGlobalAmbientLight(GetColorF(0.3f, 0.3f, 0.3f, 1.0f));

	// デフォルトライトの影響を抑える
	SetLightEnable(FALSE);

	// ディレクショナルライトの作成
	// 上から下
	lightHandle_ = CreateDirLightHandle(VGet(0.0f, 1.0f, 0.0f));

	// 右から左
	lightHandle2_ = CreateDirLightHandle(VGet(1.0f, 0.0f, 0.0f));

	// 左から右
	lightHandle3_ = CreateDirLightHandle(VGet(-1.0f, 0.0f, 0.0f));

	// 右上から中央
	lightHandle4_ = CreateDirLightHandle(VGet(-1.0f, 1.0f, 0.0f));

	// 左上から中央
	lightHandle5_ = CreateDirLightHandle(VGet(1.0f, 1.0f, 0.0f));

	// 各ライトの有効化と色設定
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
	transform_.scl = { 0.1f,0.1f,0.1f };
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Identity();
	transform_.pos = INIT_POS;
	transform_.Update();
}

void Stage::InitCollider(void)
{
	// DxLib側の衝突判定をセットアップ
	MV1SetupCollInfo(transform_.modelId);

	// モデルのコライダー
	ColliderModel * colModel = 
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
