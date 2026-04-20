#include <DxLib.h>
#include "Stage.h"

Stage::Stage()
{
}

Stage::~Stage()
{
}

void Stage::Init(void)
{
	// ステージモデルの読み込み
	// データパスとファイル名は要修正
	modelId_ = MV1LoadModel("Data/Model/m.mv1");

	// 位置・角度・拡縮の初期化
	pos_ = { 0.0f, 0.0f, 0.0f };
	angle_ = { 0.0f, 0.0f, 0.0f };
	scale_ = { 0.1f, 0.1f, 0.1f };


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

void Stage::Update(void)
{
	// ステージの更新処理が必要な場合はここに追加
	MV1SetPosition(modelId_, pos_);
	MV1SetRotationXYZ(modelId_, angle_);
	MV1SetScale(modelId_, scale_);

	// ライトはInit時にハンドルで作成済み
}

void Stage::Draw(void)
{
	// ステージモデルの描画
	MV1DrawModel(modelId_);

	// フォグ
	//SetFogEnable(true);

	// フォグの色と距離の設定
	//SetFogColor(100, 110, 1100);
	//SetFogStartEnd(100.0f, 500.0f);


}

void Stage::Release(void)
{
	// ステージモデルの解放
	MV1DeleteModel(modelId_);

	// ライトの解放
	DeleteLightHandle(lightHandle_);
	DeleteLightHandle(lightHandle2_);
	DeleteLightHandle(lightHandle3_);
	DeleteLightHandle(lightHandle4_);
	DeleteLightHandle(lightHandle5_);
}
