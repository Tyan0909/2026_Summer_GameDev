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

	const auto padNo = InputManager::JOYPAD_NO::PAD1;

	// Xボタン
	if (
		ins.IsTrgDown(KEY_INPUT_SPACE) ||
		ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::LEFT)
		)
	{
		scene.SetCarryMoney(0);
		scene.SetGameResult(SceneManager::GAME_RESULT::NONE);
		scene.SetPhotoCount(0);
		scene.SetLastPhotoScore(0);
		scene.ChangeScene(SceneManager::SCENE_ID::TITLE);
		return;
	}

	// Aボタン
	if (
		ins.IsTrgDown(KEY_INPUT_RETURN) ||
		ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::DOWN)
		)
	{
		if (scene.GetGameResult() == SceneManager::GAME_RESULT::CLEAR)
		{
			scene.ChangeScene(SceneManager::SCENE_ID::BUYSELECT);
		}
		else
		{
			scene.SetCarryMoney(0);
			scene.SetGameResult(SceneManager::GAME_RESULT::NONE);
			scene.SetPhotoCount(0);
			scene.SetLastPhotoScore(0);
			scene.ChangeScene(SceneManager::SCENE_ID::TITLE);
		}
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
		DrawString(200, 370, "A BUTTON : BUY SELECT", GetColor(255, 255, 255));
	}
	else
	{
		DrawString(200, 300, "A BUTTON : TITLE", GetColor(255, 255, 255));
	}

	DrawString(200, 420, "X BUTTON : TITLE", GetColor(255, 255, 255));
}

void Result::Release(void) {}