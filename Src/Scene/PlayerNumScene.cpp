#include "PlayerNumScene.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"

PlayerNumScene::PlayerNumScene(void)
{
}

PlayerNumScene::~PlayerNumScene(void)
{
}

void PlayerNumScene::Init(void)
{
}

void PlayerNumScene::Update(void)
{
	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	// スペースキーでゲームシーンへ遷移
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::EXAMPLE);
	}

	// Enterキーでタイトルシーンへ遷移
	if (ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
}

void PlayerNumScene::Draw(void)
{
	// プレイヤー数選択シーンの描画
	DrawString(200, 200, "プレイヤー数選択シーン", GetColor(255, 255, 255));
}

void PlayerNumScene::Release(void)
{
}
