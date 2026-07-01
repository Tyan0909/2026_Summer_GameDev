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

	void SetPlayerScore(const std::vector<int>& score){playerScore_ = score;}
	const std::vector<int>& GetPlayerScore() const{return playerScore_;}

	// 追加: 購入したアイテムの型(ID列)を保存 / 取得
	void SetPurchasedItemsPerPlayer(const std::vector<std::vector<int>>& items){purchasedItemsPerPlayer_ = items;}
	const std::vector<std::vector<int>>&GetPurchasedItemsPerPlayer() const{return purchasedItemsPerPlayer_;}

	//プレイヤーごとの所持金を保存 / 取得
	void SetPlayerMoney(const std::vector<int>& money){playerMoney_ = money;}
	const std::vector<int>&GetPlayerMoney() const{return playerMoney_;}


	// 追加: 購入したアイテムの型(ID列)を保存 / 取得
	void SetPurchasedItemTypes(const std::vector<int>& items);
	const std::vector<int>& GetPurchasedItemTypes() const;

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

	// プレイヤーごとの所持金（最低保証を含む）
	std::vector<int> playerMoney_;
	// プレイヤーごとのスコア
	std::vector<int> playerScore_;
	// プレイヤーごとの購入アイテムリスト（アイテムID列）
	std::vector<std::vector<int>> purchasedItemsPerPlayer_;
	std::vector<int> selectedPlayerNums_;
	bool isSplitScreenEnabled_ = true;

	// 追加: 購入したアイテムの型(ID列)を保存 / 取得
	std::vector<int> purchasedItemTypes_;

	GAME_RESULT gameResult_ = GAME_RESULT::NONE;
	int photoCount_ = 0;
	int lastPhotoScore_ = 0;

	int pauseMenuIndex_ = 0;

	

public:

	void SetPlayerNum(int num) { playerNum_ = num; }
	int GetPlayerNum(void) const { return playerNum_; }

	void SetSelectedPlayerNums(const std::vector<int>& types) { selectedPlayerNums_ = types; }
	const std::vector<int>& GetSelectedPlayerNums() const { return selectedPlayerNums_; }
};