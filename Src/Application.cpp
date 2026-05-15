#include <DxLib.h>
#include "Manager/InputManager.h"
#include "Manager/SceneManager.h"
#include "Manager/ResourceManager.h"
#include "Application.h"

Application* Application::instance_ = nullptr;

const std::string Application::PATH_DATA = "Data/";
const std::string Application::PATH_IMAGE = "Data/Image/";
const std::string Application::PATH_MODEL = "Data/Model/";
const std::string Application::PATH_EFFECT = "Data/Effect/";

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

	// 乱数のシード値を設定する
	DATEDATA date;

	// 現在時刻を取得する
	GetDateTime(&date);

	// 乱数の初期値を設定する
	// 設定する数値によって、ランダムの出方が変わる
	SRand(date.Year + date.Mon + date.Day + date.Hour + date.Min + date.Sec);


	// リソース管理初期化
	ResourceManager::CreateInstance();

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
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{

		inputManager.Update();
		sceneManager.Update();

		sceneManager.Draw();

		// フレームレート数を表示
		// 画面左上に表示
		DrawFormatString(450, 620, GetColor(255, 255, 255), "FPS: %.15f", GetFPS());

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

void Application::Destroy(void)
{

	// シーン管理解放
	SceneManager::GetInstance().Destroy();

	// 入力制御解放
	InputManager::GetInstance().Destroy();

	// リソース管理解放
	ResourceManager::GetInstance().Destroy();

	// DxLib終了
	if (DxLib_End() == -1)
	{
		isReleaseFail_ = true;
	}

	// インスタンスのメモリ解放
	delete instance_;

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
}
