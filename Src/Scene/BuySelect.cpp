#include "BuySelect.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"

BuySelect::BuySelect(void)
{
}

BuySelect::~BuySelect(void)
{
}

void BuySelect::Init(void)
{
}

void BuySelect::Update(void)
{
	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	// スペースキーでゲームシーンへ遷移
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::GAME);
	}
}

void BuySelect::Draw(void)
{
	// アイテム選択・購入シーンの描画
	DrawString(200, 200, "アイテム選択・購入シーン", GetColor(255, 255, 255));
}

void BuySelect::Release(void)
{

}
