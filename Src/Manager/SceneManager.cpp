#include <chrono>
#include <DxLib.h>
#include "../Common/Fader.h"
#include "../Scene/TitleScene.h"
#include "../Scene/GameScene.h"
#include "../Scene/PlayerNumScene.h"
#include "../Scene/BuySelect.h"
#include "../Scene/ExampleScene.h"
#include "../Scene/Result.h"
#include "Camera.h"
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
	if (scene_ == nullptr) return;

	auto nowTime = std::chrono::system_clock::now();
	deltaTime_ = static_cast<float>(
		std::chrono::duration_cast<std::chrono::nanoseconds>(nowTime - preTime_).count() / 1000000000.0);
	preTime_ = nowTime;

	fader_->Update();
	if (isSceneChanging_)
	{
		Fade();
	}
	else
	{
		scene_->Update();
	}

	camera_->Update();
}

void SceneManager::Draw(void)
{
	SetDrawScreen(DX_SCREEN_BACK);
	ClearDrawScreen();

	camera_->SetBeforeDraw();

	scene_->Draw();

	camera_->DrawDebug();

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
	waitSceneId_ = nextId;
	fader_->SetFade(Fader::STATE::FADE_OUT);
	isSceneChanging_ = true;
}

SceneManager::SceneManager(void)
{
	sceneId_ = SCENE_ID::NONE;
	waitSceneId_ = SCENE_ID::NONE;

	scene_ = nullptr;
	fader_ = nullptr;

	isSceneChanging_ = false;

	deltaTime_ = 1.0f / 60.0f;

	camera_ = nullptr;

	// š”O‚Ì‚½‚ß–¾Ž¦‰Šú‰»
	carryMoney_ = 0;
	playerNum_ = 0;
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