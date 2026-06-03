#pragma once
#include <chrono>
#include <vector>
class SceneBase;
class Fader;
class Camera;

class SceneManager
{
public:
	// 背景色
	static constexpr int BACKGROUND_COLOR_R = 0;
	static constexpr int BACKGROUND_COLOR_G = 139;
	static constexpr int BACKGROUND_COLOR_B = 139;

	// シーン管理ID
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

	// carryMoney_ は「ゲーム中に撮影で獲得した合計スコア」として扱う
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

	// 追加: 購入したアイテムの型(ID列)を保存 / 取得
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

	std::chrono::system_clock::time_point preTime_;
	float deltaTime_;

	SceneManager(void);
	SceneManager(const SceneManager& instance) = default;
	~SceneManager(void) = default;

	void ResetDeltaTime(void);
	void DoChangeScene(SCENE_ID sceneId);
	void Fade(void);

	int playerNum_ = 0;

	// 「ゲーム中に撮影で獲得した合計スコア」
	int carryMoney_ = 0;

	std::vector<int> selectedPlayerNums_;
	bool isSplitScreenEnabled_ = true;

	GAME_RESULT gameResult_ = GAME_RESULT::NONE;
	int photoCount_ = 0;
	int lastPhotoScore_ = 0;

	std::vector<int> purchasedItemTypes_; // 追加: BuySelect の購入アイテムを int(ID)で保持

public:

	void SetPlayerNum(int num) { playerNum_ = num; }
	int GetPlayerNum(void) const { return playerNum_; }

	void SetSelectedPlayerNums(const std::vector<int>& types) { selectedPlayerNums_ = types; }
	const std::vector<int>& GetSelectedPlayerNums() const { return selectedPlayerNums_; }
};