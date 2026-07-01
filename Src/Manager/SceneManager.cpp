#include <chrono>
#include <cstring>
#include <DxLib.h>
#include "../Application.h"
#include "../Common/Fader.h"
#include "../Scene/TitleScene.h"
#include "../Scene/GameScene.h"
#include "../Scene/PlayerNumScene.h"
#include "../Scene/BuySelect.h"
#include "../Scene/ExampleScene.h"
#include "../Scene/Result.h"
#include "../Scene/LoadingScene.h"
#include "Camera.h"
#include "InputManager.h"
#include "SceneManager.h"

SceneManager* SceneManager::instance_ = nullptr;

void SceneManager::CreateInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new SceneManager();
	}
	instance_->Init();
}

SceneManager& SceneManager::GetInstance(void)
{
	return *instance_;
}

void SceneManager::Init(void)
{
	sceneId_ = SCENE_ID::TITLE;
	waitSceneId_ = SCENE_ID::NONE;

	fader_ = new Fader();
	fader_->Init();

	camera_ = new Camera();
	camera_->Init();

	scene_ = new TitleScene();
	scene_->Init();

	isSceneChanging_ = false;
	isPaused_ = false;
	ResetPauseMenu();

	preTime_ = std::chrono::system_clock::now();

	Init3D();

	DoChangeScene(SCENE_ID::TITLE);
}

void SceneManager::Init3D(void)
{
	SetBackgroundColor(
		BACKGROUND_COLOR_R,
		BACKGROUND_COLOR_G,
		BACKGROUND_COLOR_B);

	SetUseZBuffer3D(true);
	SetWriteZBuffer3D(true);
	SetUseBackCulling(true);

	SetUseLighting(true);
	ChangeLightTypeDir({ 0.00f, -1.00f, 1.00f });
}

void SceneManager::Update(void)
{
	if (scene_ == nullptr)
	{
		return;
	}

	auto nowTime = std::chrono::system_clock::now();
	deltaTime_ = static_cast<float>(
		std::chrono::duration_cast<std::chrono::nanoseconds>(nowTime - preTime_).count() / 1000000000.0);
	preTime_ = nowTime;

	fader_->Update();

	if (isSceneChanging_)
	{
		isPaused_ = false;
		ResetPauseMenu();
		Fade();
		return;
	}

	InputManager& input = InputManager::GetInstance();

	if (input.IsTrgDown(KEY_INPUT_ESCAPE))
	{
		isPaused_ = !isPaused_;
		ResetPauseMenu();
		ResetDeltaTime();
		return;
	}

	if (isPaused_)
	{
		UpdatePauseMenu();
		return;
	}

	scene_->Update();
	camera_->Update();
}

void SceneManager::Draw(void)
{
	SetDrawScreen(DX_SCREEN_BACK);
	ClearDrawScreen();

	camera_->SetBeforeDraw();

	scene_->Draw();

	camera_->DrawDebug();

	if (isPaused_)
	{
		DrawPauseOverlay();
	}

	fader_->Draw();
}

void SceneManager::Destroy(void)
{
	scene_->Release();
	delete scene_;

	delete fader_;

	camera_->Release();
	delete camera_;

	delete instance_;
}

void SceneManager::ChangeScene(SCENE_ID nextId)
{
	isPaused_ = false;
	ResetPauseMenu();
	waitSceneId_ = nextId;
	fader_->SetFade(Fader::STATE::FADE_OUT);
	isSceneChanging_ = true;
}

void SceneManager::SetPurchasedItemTypes(const std::vector<int>& items)
{
	purchasedItemTypes_ = items;
}

const std::vector<int>&SceneManager::GetPurchasedItemTypes() const
{
	return purchasedItemTypes_;
}

SceneManager::SceneManager(void)
{
	sceneId_ = SCENE_ID::NONE;
	waitSceneId_ = SCENE_ID::NONE;

	scene_ = nullptr;
	fader_ = nullptr;

	isSceneChanging_ = false;
	isPaused_ = false;

	deltaTime_ = 1.0f / 60.0f;

	camera_ = nullptr;

	carryMoney_ = 0;
	playerNum_ = 0;
	isSplitScreenEnabled_ = true;
	pauseMenuIndex_ = 0;
}

void SceneManager::ResetDeltaTime(void)
{
	deltaTime_ = 0.016f;
	preTime_ = std::chrono::system_clock::now();
}

void SceneManager::DoChangeScene(SCENE_ID sceneId)
{
	sceneId_ = sceneId;

	if (scene_ != nullptr)
	{
		scene_->Release();
		delete scene_;
	}

	switch (sceneId_)
	{
	case SCENE_ID::TITLE:
		scene_ = new TitleScene();
		break;
	case SCENE_ID::PLAYERNUMBERSELECT:
		scene_ = new PlayerNumScene();
		break;
	case SCENE_ID::EXAMPLE:
		scene_ = new ExampleScene();
		break;
	case SCENE_ID::BUYSELECT:
		scene_ = new BuySelect();
		break;
	case SCENE_ID::LOADING:
		scene_ = new LoadingScene();
		break;
	case SCENE_ID::GAME:
		scene_ = new GameScene();
		break;
	case SCENE_ID::RESULT:
		scene_ = new Result();
		break;
	default:
		break;
	}

	scene_->Init();

	ResetDeltaTime();

	waitSceneId_ = SCENE_ID::NONE;
	isPaused_ = false;
	ResetPauseMenu();
}

void SceneManager::Fade(void)
{
	Fader::STATE fState = fader_->GetState();
	switch (fState)
	{
	case Fader::STATE::FADE_IN:
		if (fader_->IsEnd())
		{
			fader_->SetFade(Fader::STATE::NONE);
			isSceneChanging_ = false;
		}
		break;
	case Fader::STATE::FADE_OUT:
		if (fader_->IsEnd())
		{
			DoChangeScene(waitSceneId_);
			fader_->SetFade(Fader::STATE::FADE_IN);
		}
		break;
	}
}

void SceneManager::ResetPauseMenu(void)
{
	pauseMenuIndex_ = static_cast<int>(PAUSE_MENU_ITEM::RESUME);
}

void SceneManager::UpdatePauseMenu(void)
{
	InputManager& input = InputManager::GetInstance();
	const int menuCount = static_cast<int>(PAUSE_MENU_ITEM::MAX);

	if (input.IsTrgDown(KEY_INPUT_UP))
	{
		pauseMenuIndex_--;
		if (pauseMenuIndex_ < 0)
		{
			pauseMenuIndex_ = menuCount - 1;
		}
	}

	if (input.IsTrgDown(KEY_INPUT_DOWN))
	{
		pauseMenuIndex_++;
		if (pauseMenuIndex_ >= menuCount)
		{
			pauseMenuIndex_ = 0;
		}
	}

	if (input.IsTrgDown(KEY_INPUT_RETURN))
	{
		ExecutePauseMenu();
	}
}

void SceneManager::ExecutePauseMenu(void)
{
	switch (static_cast<PAUSE_MENU_ITEM>(pauseMenuIndex_))
	{
	case PAUSE_MENU_ITEM::RESUME:
		isPaused_ = false;
		ResetPauseMenu();
		ResetDeltaTime();
		break;

	case PAUSE_MENU_ITEM::TITLE:
		ChangeScene(SCENE_ID::TITLE);
		break;

	case PAUSE_MENU_ITEM::EXIT:
		Application::GetInstance().RequestEnd();
		break;

	case PAUSE_MENU_ITEM::MAX:
	default:
		break;
	}
}

void SceneManager::DrawPauseOverlay(void) const
{
	int screenWidth = 0;
	int screenHeight = 0;
	GetDrawScreenSize(&screenWidth, &screenHeight);


	const int backColor = GetColor(0, 0, 0);
	const int frameColor = GetColor(255, 255, 255);
	const int titleColor = GetColor(255, 255, 0);
	const int textColor = GetColor(255, 255, 255);
	const int selectedTextColor = GetColor(0, 0, 0);
	const int selectedBackColor = GetColor(255, 255, 255);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	DrawBox(0, 0, screenWidth, screenHeight, backColor, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	const int boxWidth = 460;
	const int boxHeight = 300;
	const int left = (screenWidth - boxWidth) / 2;
	const int top = (screenHeight - boxHeight) / 2;
	const int right = left + boxWidth;
	const int bottom = top + boxHeight;

	DrawBox(left, top, right, bottom, frameColor, FALSE);

	const char* pauseText = "PAUSE";
	const int pauseTextWidth = GetDrawStringWidth(
		pauseText,
		static_cast<int>(std::strlen(pauseText)));

	DrawString(
		left + (boxWidth - pauseTextWidth) / 2,
		top + 30,
		pauseText,
		titleColor);

	const char* menuItems[] =
	{
		"RESUME",
		"TITLE",
		"EXIT",
	};

	const int menuStartY = top + 90;
	const int menuStepY = 42;
	const int menuAreaLeft = left + 90;
	const int menuAreaRight = right - 90;

	for (int i = 0; i < static_cast<int>(PAUSE_MENU_ITEM::MAX); i++)
	{
		const bool isSelected = (i == pauseMenuIndex_);
		const int itemY = menuStartY + i * menuStepY;
		const char* itemText = menuItems[i];
		const int itemTextWidth = GetDrawStringWidth(
			itemText,
			static_cast<int>(std::strlen(itemText)));
		const int textX = left + (boxWidth - itemTextWidth) / 2;

		if (isSelected)
		{
			DrawBox(
				menuAreaLeft,
				itemY - 4,
				menuAreaRight,
				itemY + 24,
				selectedBackColor,
				TRUE);
		}

		DrawString(
			textX,
			itemY,
			itemText,
			isSelected ? selectedTextColor : textColor);
	}

	const char* guideText1 = "UP / DOWN : SELECT";
	const char* guideText2 = "ENTER : DECIDE";
	const char* guideText3 = "ESC : RESUME";

	DrawString(left + 30, bottom - 80, guideText1, textColor);
	DrawString(left + 30, bottom - 55, guideText2, textColor);
	DrawString(left + 30, bottom - 30, guideText3, textColor);
}