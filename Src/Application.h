#pragma once
#include <string>

class Application
{

public:

	// スクリーンサイズ
	static constexpr int SCREEN_SIZE_X = 1280;
	static constexpr int SCREEN_SIZE_Y = 720;

	// FPS関連
	static constexpr float TARGET_FPS = 60.0f;
	static constexpr float FRAME_TIME = 1000.0f / TARGET_FPS;

	// データパス関連
	//-------------------------------------------
	static const std::string PATH_DATA;
	static const std::string PATH_IMAGE;
	static const std::string PATH_MODEL;
	static const std::string PATH_EFFECT;
	static const std::string PATH_SOUND;
	//-------------------------------------------

	// インスタンスを明示的に生成
	static void CreateInstance(void);

	// インスタンスの取得
	static Application& GetInstance(void);

	// 初期化
	void Init(void);

	// ゲームループの開始
	void Run(void);

	// リソースの破棄
	void Destroy(void);

	// 初期化失敗の判定
	bool IsInitFail(void) const;

	// 解放失敗の判定
	bool IsReleaseFail(void) const;

	void RequestScreenshot(void);
	bool HasScreenshot(void) const;
	int GetScreenshotHandle(void) const;

	void RequestEnd(void);

private:

	// 静的インスタンス
	static Application* instance_;

	// 初期化失敗
	bool isInitFail_;

	// 解放失敗
	bool isReleaseFail_;

	Application(void);
	Application(const Application& instance) = default;
	~Application(void) = default;

	// ポーズ管理
	bool isPaused_ = false;

	bool isScreenshotRequested_;
	bool hasScreenshot_;
	int screenshotHandle_;
	bool isEndRequested_;

};