#pragma once
#include <chrono>
#include <vector>

class SceneBase;
class Fader;
class Camera;

class SceneManager
{
public:
	static constexpr int BACKGROUND_COLOR_R = 20;
	static constexpr int BACKGROUND_COLOR_G = 30;
	static constexpr int BACKGROUND_COLOR_B = 70;

	enum class SCENE_ID
	{
		NONE,
		TITLE,
		PLAYERNUMBERSELECT,
		EXAMPLE,
		BUYSELECT,
		LOADING,
		GAME,
		RESULT,
	};

	enum class GAME_RESULT
	{
		NONE,
		CLEAR,
		GAMEOVER,
	};

	enum class PAUSE_MENU_ITEM
	{
		RESUME,
		TITLE,
		EXIT,
		MAX,
	};

	static void CreateInstance(void);
	static SceneManager& GetInstance(void);

	void Init(void);
	void Init3D(void);
	void Update(void);
	void Draw(void);
	void Destroy(void);

	void ChangeScene(SCENE_ID nextId);

	SCENE_ID GetSceneID(void) const { return sceneId_; }
	float GetDeltaTime(void) const { return deltaTime_; }
	Camera* GetCamera(void) const { return camera_; }

	void SetCarryMoney(int money) { carryMoney_ = money; }
	int GetCarryMoney(void) const { return carryMoney_; }

	void SetSplitScreenEnabled(bool isEnabled) { isSplitScreenEnabled_ = isEnabled; }
	bool IsSplitScreenEnabled(void) const { return isSplitScreenEnabled_; }

	void SetGameResult(GAME_RESULT result) { gameResult_ = result; }
	GAME_RESULT GetGameResult(void) const { return gameResult_; }

	void SetPhotoCount(int count) { photoCount_ = count; }
	int GetPhotoCount(void) const { return photoCount_; }

	void SetLastPhotoScore(int score) { lastPhotoScore_ = score; }
	int GetLastPhotoScore(void) const { return lastPhotoScore_; }

	void SetPaused(bool isPaused) { isPaused_ = isPaused; }
	bool IsPaused(void) const { return isPaused_; }

	// í«â¡: çwì¸ÇµÇΩÉAÉCÉeÉÄÇÃå^(IDóÒ)Çï€ë∂ / éÊìæ
	void SetPurchasedItemTypes(const std::vector<int>& types) { purchasedItemTypes_ = types; }
	const std::vector<int>& GetPurchasedItemTypes() const { return purchasedItemTypes_; }

private:
	static SceneManager* instance_;

	SCENE_ID sceneId_;
	SCENE_ID waitSceneId_;

	Fader* fader_;
	SceneBase* scene_;
	Camera* camera_;
	bool isSceneChanging_;
	bool isPaused_;

	std::chrono::system_clock::time_point preTime_;
	float deltaTime_;

	SceneManager(void);
	SceneManager(const SceneManager& instance) = default;
	~SceneManager(void) = default;

	void ResetDeltaTime(void);
	void DoChangeScene(SCENE_ID sceneId);
	void Fade(void);
	void DrawPauseOverlay(void) const;
	void ResetPauseMenu(void);
	void UpdatePauseMenu(void);
	void ExecutePauseMenu(void);

	int playerNum_ = 0;
	int carryMoney_ = 0;

	std::vector<int> selectedPlayerNums_;
	bool isSplitScreenEnabled_ = true;

	GAME_RESULT gameResult_ = GAME_RESULT::NONE;
	int photoCount_ = 0;
	int lastPhotoScore_ = 0;

	int pauseMenuIndex_ = 0;

	std::vector<int> purchasedItemTypes_;

public:

	void SetPlayerNum(int num) { playerNum_ = num; }
	int GetPlayerNum(void) const { return playerNum_; }

	void SetSelectedPlayerNums(const std::vector<int>& types) { selectedPlayerNums_ = types; }
	const std::vector<int>& GetSelectedPlayerNums() const { return selectedPlayerNums_; }
};