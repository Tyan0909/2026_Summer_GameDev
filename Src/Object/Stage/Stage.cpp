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
	scale_ = { 0.001f, 0.001f, 0.001f };

}

void Stage::Update(void)
{
	// ステージの更新処理が必要な場合はここに追加

	// 発光色を白に設定
	COLOR_F emiColor;
	emiColor.r = 1.0f;
	emiColor.g = 1.0f;
	emiColor.b = 1.0f;
	emiColor.a = 1.0f;

	MV1SetMaterialEmiColor(modelId_, 0, emiColor);
}

void Stage::Draw(void)
{
	// ステージモデルの描画
	MV1DrawModel(modelId_);

	
}

void Stage::Release(void)
{
	// ステージモデルの解放
	MV1DeleteModel(modelId_);
}
