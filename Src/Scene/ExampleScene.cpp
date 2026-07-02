#include "ExampleScene.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Application.h"

ExampleScene::ExampleScene(void)
{
}

ExampleScene::~ExampleScene(void)
{
}

void ExampleScene::Init(void)
{
	exampleImg_ = 0;
}

void ExampleScene::Update(void)
{
	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();
	// スペースキーでゲームシーンへ遷移
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::BUYSELECT);
	}

	// Enterキーでタイトルシーンへ遷移
	if (ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
}

void ExampleScene::Draw(void)
{
	// 説明シーンの描画
	DrawString(200, 200, "説明シーン", GetColor(255, 255, 255));
}

void ExampleScene::Release(void)
{
}
