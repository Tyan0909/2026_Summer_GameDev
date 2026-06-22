#include <DxLib.h>
#include "Manager/InputManager.h"
#include "Manager/SceneManager.h"
#include "Manager/ResourceManager.h"
#include "Manager/SoundManager.h"
#include <EffekseerForDXLib.h>
#include "Application.h"

Application* Application::instance_ = nullptr;

const std::string Application::PATH_DATA = "Data/";
const std::string Application::PATH_IMAGE = "Data/Image/";
const std::string Application::PATH_MODEL = "Data/Model/";
const std::string Application::PATH_EFFECT = "Data/Effect/";
const std::string Application::PATH_SOUND = "Data/Sound/";

void Application::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new Application();
	}
	instance_->Init();
}

Application& Application::GetInstance(void)
{
	return *instance_;
}

void Application::Init(void)
{
	// アプリケーションの初期設定
	// ウィンドウタイトル
	SetWindowText("スクープ最前線");

	// ウィンドウサイズ
	SetGraphMode(SCREEN_SIZE_X, SCREEN_SIZE_Y, 32);
	ChangeWindowMode(true);

	// DxLibの初期化
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	isInitFail_ = false;
	if (DxLib_Init() == -1)
	{
		isInitFail_ = true;
		return;
	}

	// Effekseer初期化
	if (Effekseer_Init(8000) == -1)
	{
		isInitFail_ = true;
		return;
	}

	screenshotHandle_ = MakeScreen(SCREEN_SIZE_X, SCREEN_SIZE_Y, TRUE);
	isScreenshotRequested_ = false;
	hasScreenshot_ = false;
	isEndRequested_ = false;

	// 乱数のシード値を設定する
	DATEDATA date;

	// 現在時刻を取得する
	GetDateTime(&date);

	// 乱数の初期値を設定する
	SRand(date.Year + date.Mon + date.Day + date.Hour + date.Min + date.Sec);

	// リソース管理初期化
	ResourceManager::CreateInstance();

	// サウンド管理初期化
	SoundManager::CreateInstance();

	// 入力制御初期化
	SetUseDirectInputFlag(true);
	InputManager::CreateInstance();

	// シーン管理初期化
	SceneManager::CreateInstance();
}

void Application::Run(void)
{
	InputManager& inputManager = InputManager::GetInstance();
	SceneManager& sceneManager = SceneManager::GetInstance();

	// 前フレームのミリ秒を取得
	int prevTime = GetNowCount();

	// ゲームループ
	while (ProcessMessage() == 0 && !isEndRequested_)
	{
		inputManager.Update();
		sceneManager.Update();

		sceneManager.Draw();

		// フレームレート数を表示
		// 画面左上に表示
		DrawFormatString(450, 620, GetColor(255, 255, 255), "FPS: %.15f", GetFPS());

		if (isScreenshotRequested_ && screenshotHandle_ != -1)
		{
			GetDrawScreenGraph(
				0,
				0,
				SCREEN_SIZE_X - 1,
				SCREEN_SIZE_Y - 1,
				screenshotHandle_,
				FALSE);

			hasScreenshot_ = true;
			isScreenshotRequested_ = false;
		}

		ScreenFlip();

		// フレームレート制御
		int currentTime = GetNowCount();

		// 経過時間を計算
		int elapsedTime = currentTime - prevTime;

		// 待機時間を計算
		int waitTime = static_cast<int>(FRAME_TIME) - elapsedTime;

		// 待機時間が正の値なら待機する
		if (waitTime > 0)
		{
			DxLib::WaitTimer(waitTime);
		}

		// 現在の時間を前フレームの時間として保存
		prevTime = GetNowCount();
	}
}

void Application::RequestScreenshot(void)
{
	isScreenshotRequested_ = true;
}

bool Application::HasScreenshot(void) const
{
	return hasScreenshot_;
}

int Application::GetScreenshotHandle(void) const
{
	return screenshotHandle_;
}

void Application::RequestEnd(void)
{
	isEndRequested_ = true;
}

void Application::Destroy(void)
{
	// シーン管理解放
	SceneManager::GetInstance().Destroy();

	// 入力制御解放
	InputManager::GetInstance().Destroy();

	// サウンド管理破棄
	SoundManager::GetInstance().Destroy();

	// リソース管理解放
	ResourceManager::GetInstance().Destroy();

	if (screenshotHandle_ != -1)
	{
		DeleteGraph(screenshotHandle_);
		screenshotHandle_ = -1;
	}

	Effkseer_End();

	// DxLib終了
	if (DxLib_End() == -1)
	{
		isReleaseFail_ = true;
	}

	// インスタンスのメモリ解放
	delete instance_;
	instance_ = nullptr;
}

bool Application::IsInitFail(void) const
{
	return isInitFail_;
}

bool Application::IsReleaseFail(void) const
{
	return isReleaseFail_;
}

Application::Application(void)
{
	isInitFail_ = false;
	isReleaseFail_ = false;
	isScreenshotRequested_ = false;
	hasScreenshot_ = false;
	screenshotHandle_ = -1;
	isEndRequested_ = false;
}