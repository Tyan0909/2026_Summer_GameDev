#include <cmath>
#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/Grid.h"
#include "../Object/UIInput.h"
#include "../Manager/Camera.h"
#include "../Application.h"
#include "TitleScene.h"

TitleScene::TitleScene(void) : SceneBase()
{
	debugGrid_ = nullptr;
}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{
	// カメラモード変更
	Camera* camera = SceneManager::GetInstance().GetCamera();
	camera->ChangeMode(Camera::MODE::FREE);

	// デバッグ用グリッド
	debugGrid_ = new Grid();
	debugGrid_->Init();

	// リソース読み込み
	logoHandle_ = LoadGraph("data/Image/Title/Title logo.png");
	pointerHandle_ = LoadGraph("data/Image/Title/pointer.png");
	shutterSE_ = LoadSoundMem("data/Sound/shutter.wav");
	pressHandle_ = LoadGraph("data/Image/Title/press_space.png");

	// 演出初期化
	shutterScale_ = 1.0f;
	isShutter_ = false;
	flashAlpha_ = 0;
	fadeAlpha_ = 0;
	pointerAnim_ = 0.0f;
	logoAlpha_ = 0;

	// ポインタ位置
	pointerPos_ = VGet(200, 200, 0);
	targetPos_ = VGet(640, 360, 0);

}

void TitleScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	// Enterキー：分割なし直接開始
	if (ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		scene.SetSplitScreenEnabled(false);
		scene.ChangeScene(SceneManager::SCENE_ID::GAME);
		return;
	}

	// Spaceキー：通常開始
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		scene.SetSplitScreenEnabled(true);

		isShutter_ = true;
		shutterScale_ = 1.0f;
		flashAlpha_ = 255;

		PlaySoundMem(shutterSE_, DX_PLAYTYPE_BACK);
	}

	// シャッター演出
	if (isShutter_)
	{

		shutterScale_ -= 0.08f;

		// 黒フェード増加
		fadeAlpha_ += 12;

		if (shutterScale_ <= 0.0f)
		{
			shutterScale_ = 0.0f;
			isShutter_ = false;

			scene.ChangeScene(SceneManager::SCENE_ID::PLAYERNUMBERSELECT);
		}
	}

	// フラッシュ演出
	if (flashAlpha_ > 0)
	{
		flashAlpha_ -= 20;

		if (flashAlpha_ < 0)
		{
			flashAlpha_ = 0;
		}
	}

	if (GetRand(100) < 2)
	{
		/*logoAlpha_ = 120 + GetRand(135);*/
		logoAlpha_ = 40;
	}
	else
	{
		/*logoAlpha_ += 10;*/
		logoAlpha_ += 25;

		if (logoAlpha_ > 255)
		{
			logoAlpha_ = 255;
		}
	}

	// ポインタ追従
	pointerPos_.x += (targetPos_.x - pointerPos_.x) * 0.1f;
	pointerPos_.y += (targetPos_.y - pointerPos_.y) * 0.1f;

	pointerAnim_ += 0.5f;

	debugGrid_->Update();
}

void TitleScene::Draw(void)
{
	// 3D描画
	debugGrid_->Draw();

	// 背景暗くする
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);

	DrawBox(
		0,
		0,
		1280,
		720,
		GetColor(0, 0, 0),
		TRUE
	);



	// ロゴ
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, logoAlpha_);
	DrawRotaGraph(
		520,
		260,
		0.7,
		0.0,
		logoHandle_,
		TRUE
	);
	
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// ポインタ
	DrawRotaGraph(
		(int)pointerPos_.x,
		(int)pointerPos_.y,
		1.0,
		0.0,
		pointerHandle_,
		TRUE
	);

	// フラッシュ
	if (flashAlpha_ > 0)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, flashAlpha_);

		DrawBox(
			0,
			0,
			1280,
			720,
			GetColor(255,255,255),
			TRUE
		);

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// UI
	DrawString(200, 200, "タイトルシーン", GetColor(255, 255, 255));
	DrawString(200, 240, "SPACE : 通常開始", GetColor(255, 255, 255));
	DrawString(200, 280, "ENTER : 分割なしで直接ゲーム開始", GetColor(255, 255, 255));

	// PRESS SPACE 点滅
	int textAlpha =
		(int)((std::sin(pointerAnim_ * 0.05f) * 0.5f + 0.5f) * 255);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, textAlpha);

	DrawRotaGraph(
		500,
		520,
		0.35,
		0.0,
		pressHandle_,
		TRUE
	);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

}

void TitleScene::Release(void)
{
	// グリッド解放
	delete debugGrid_;
	debugGrid_ = nullptr;

	// リソース解放
	DeleteGraph(logoHandle_);
	DeleteGraph(pointerHandle_);
	DeleteSoundMem(shutterSE_);
	DeleteGraph(pressHandle_);
}