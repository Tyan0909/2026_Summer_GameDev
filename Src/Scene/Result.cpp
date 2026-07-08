#include "Result.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include <DxLib.h>

Result::Result(void) {}
Result::~Result(void) {}

void Result::Init(void) {}

void Result::Update(void)
{
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

    if (ins.IsTrgDown(KEY_INPUT_SPACE))
    {
        scene.SetCarryMoney(0);
        scene.SetPlayerMoney({});

        scene.ChangeScene(SceneManager::SCENE_ID::TITLE);
    }

	if (ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::BUYSELECT);
	}
}

void Result::Draw(void)
{
	SceneManager& scene = SceneManager::GetInstance();

	const int totalScore = scene.GetCarryMoney();
	const int photoCount = scene.GetPhotoCount();
	const int lastPhotoScore = scene.GetLastPhotoScore();

	const bool isClear = scene.GetGameResult() == SceneManager::GAME_RESULT::CLEAR;
	const char* title = isClear ? "GAME CLEAR" : "GAME OVER";
	const int titleColor = isClear ? GetColor(0, 255, 120) : GetColor(255, 80, 80);

	DrawString(200, 160, title, titleColor);
	DrawFormatString(200, 220, GetColor(255, 255, 0), "TOTAL SCORE : %d", totalScore);

	if (isClear)
	{
		DrawFormatString(200, 270, GetColor(255, 255, 255), "PHOTO COUNT : %d", photoCount);
		DrawFormatString(200, 310, GetColor(0, 255, 255), "LAST PHOTO : +%d", lastPhotoScore);
		DrawString(200, 370, "ENTER : BUY SELECT", GetColor(255, 255, 255));
	}
	else
	{
		DrawString(200, 300, "ENTER : TITLE", GetColor(255, 255, 255));
	}

	DrawString(200, 420, "SPACE : TITLE", GetColor(255, 255, 255));
}

void Result::Release(void) {}