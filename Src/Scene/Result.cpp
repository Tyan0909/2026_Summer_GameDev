#include "Result.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"

Result::Result(void)
{
}

Result::~Result(void)
{
}

void Result::Init(void)
{
}

void Result::Update(void)
{
	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	// スペースキーでタイトルシーンへ遷移
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::TITLE);
	}

	// Enterで残額を引き継いだ状態で購入シーンへ遷移
	if (ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::BUYSELECT);
	}
}

void Result::Draw(void)
{
	// 結果シーンの描画
	DrawString(200, 200, "結果シーン", GetColor(255, 255, 255));
}

void Result::Release(void)
{
}
