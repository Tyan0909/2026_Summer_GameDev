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
		GAME,
		RESULT,
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

	// carryMoney_ は「最低保証1500を除いた持ち越し可変分」にして統一
	void SetCarryMoney(int money) { carryMoney_ = money; }
	int GetCarryMoney(void) const { return carryMoney_; }

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

	// 「最低保証を除いた持ち越し可変分」
	int carryMoney_ = 0;

	std::vector<int> selectedPlayerNums_;

public:

	void SetPlayerNum(int num) { playerNum_ = num; }
	int GetPlayerNum(void) const { return playerNum_; }

	void SetSelectedPlayerNums(const std::vector<int>& types) { selectedPlayerNums_ = types; }
	const std::vector<int>& GetSelectedPlayerNums() const { return selectedPlayerNums_; }
};