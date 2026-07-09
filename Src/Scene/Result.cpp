#include "Result.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/PhotoManager.h"
#include <DxLib.h>

Result::Result(void) { printf("Result Constructor\n"); }
Result::~Result(void) {}

void Result::Init(void)
{
	DrawString(
		50,
		50,
		"RESULT INIT",
		GetColor(255, 255, 255)
	);

	const PhotoData* best =
		PhotoManager::GetInstance().GetBestPhoto();

	if (best)
	{
		bestPhotoHandle_ = best->graphHandle;
		bestPhotoScore_ = best->score;
		bestPhotoPlayer_ = best->playerIndex;
	}
}

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
		DrawFormatString(200, 270, GetColor(255, 255, 255), "PHOTO COUNT : %d", photoCount);
		DrawFormatString(200, 310, GetColor(0, 255, 255), "LAST PHOTO : +%d", lastPhotoScore);
		DrawString(200, 370, "ENTER : BUY SELECT", GetColor(255, 255, 255));
	}
	else
	{
		DrawString(200, 300, "ENTER : TITLE", GetColor(255, 255, 255));
	}

	DrawString(200, 420, "SPACE : TITLE", GetColor(255, 255, 255));

	if (bestPhotoHandle_ != -1)
	{
		DrawExtendGraph(
			350,
			80,
			930,
			520,
			bestPhotoHandle_,
			TRUE);

		DrawFormatString(
			430,
			540,
			GetColor(255, 255, 0),
			"BEST SHOT");

		DrawFormatString(
			430,
			570,
			GetColor(255, 255, 255),
			"SCORE : %d",
			bestPhotoScore_);

		DrawFormatString(
			430,
			600,
			GetColor(255, 255, 255),
			"PLAYER %d",
			bestPhotoPlayer_ + 1);
	}
}

void Result::Release(void)
{
	/*printf("Result Release\n");
	PhotoManager::GetInstance().Clear();

	bestPhotoHandle_ = -1;*/
}