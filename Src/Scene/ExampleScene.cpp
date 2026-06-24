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
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	const auto padNo = InputManager::JOYPAD_NO::PAD1;

	const bool isGoBuySelect =
		ins.IsTrgDown(KEY_INPUT_SPACE) ||
		ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::LEFT);

	const bool isBackTitle =
		ins.IsTrgDown(KEY_INPUT_RETURN);

	if (isGoBuySelect)
	{
		scene.ChangeScene(SceneManager::SCENE_ID::BUYSELECT);
		return;
	}

	if (isBackTitle)
	{
		scene.ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
}

void ExampleScene::Draw(void)
{
	// 説明シーンの描画
	DrawString(200, 200, "説明シーン", GetColor(255, 255, 255));
	DrawString(200, 230, "Xボタンで次へ", GetColor(255, 255, 255));
}

void ExampleScene::Release(void)
{
}
