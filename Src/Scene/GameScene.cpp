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
	// 3D設定
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	// カメラ設定
	SetCameraPositionAndTarget_UpVecY(
		VGet(0, 20, -50), // カメラ位置
		VGet(0, 0, 0)     // 注視点
	);

	// ステージ初期化
	stage.Init();
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

	stage.Update();
}

void GameScene::Draw()
{
	// ゲームシーンの描画
	DrawString(200, 200, "ゲームシーン", GetColor(255, 255, 255));
}

void GameScene::Draw3D()
{
	// 3D描画が必要な場合はここに追加
	stage.Draw();
}

void GameScene::Release()
{
	
}

