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
	//grid_ = nullptr;
}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{
	// カメラモード変更
	Camera* camera = SceneManager::GetInstance().GetCamera();
	camera->ChangeMode(Camera::MODE::FREE);

	// デバッグ用グリッドの生成

	debugGrid_ = new Grid();
	debugGrid_->Init();

}

void TitleScene::Update(void)
{
	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::PLAYERNUMBERSELECT);
	}

	// デバッグ用グリッドの更新

	debugGrid_->Update();


}

void TitleScene::Draw(void)
{
	// タイトル画面の描画
	DrawString(200, 200, "タイトルシーン", GetColor(255, 255, 255));

	// デバッグ用グリッドの描画
	debugGrid_->Draw();
}

void TitleScene::Release(void)
{

	delete debugGrid_;
	debugGrid_ = nullptr;
}

