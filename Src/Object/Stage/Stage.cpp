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

	// メイン
	SetLightDirection(VGet(-1.0f, -1.0f, 1.0f));

	// ライトの方向
	// カメラの方向に対して斜め上からの光になるように設定
	lightDirection_ = VGet(0.0f,1.0f,0.0f);
	lightDirection2_ = VGet(1.0, 1.0f, 0.0f);
	lightDirection3_ = VGet(0.0f, 1.0, 1.0f);
}

void Stage::Update(void)
{
	// ステージの更新処理が必要な場合はここに追加
	MV1SetPosition(modelId_, pos_);
	MV1SetRotationXYZ(modelId_, angle_);
	MV1SetScale(modelId_, scale_);

	ChangeLightTypeDir(lightDirection_);
	ChangeLightTypeDir(lightDirection2_);
	ChangeLightTypeDir(lightDirection3_);
}

void Stage::Draw(void)
{
	// ステージモデルの描画
	MV1DrawModel(modelId_);

	// フォグ
	SetFogEnable(true);

	// フォグの色と距離の設定
	SetFogColor(100, 110, 1100);

	SetFogStartEnd(100.0f, 500.0f);


}

void Stage::Release(void)
{
	// ステージモデルの解放
	MV1DeleteModel(modelId_);
}
