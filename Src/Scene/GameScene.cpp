#include <cmath>
#include <DxLib.h>
#include "GameScene.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"

GameScene::GameScene()
{
}

GameScene::~GameScene()
{

}

void GameScene::Init()
{
}

void GameScene::Update()
{
	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	// スペースキーで結果シーンへ遷移
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::RESULT);
	}
}

void GameScene::Draw()
{
	// ゲームシーンの描画
	DrawString(200, 200, "ゲームシーン", GetColor(255, 255, 255));
}

void GameScene::Draw3D()
{
	// 3D描画が必要な場合はここに追加
}

void GameScene::Release()
{
	
}

