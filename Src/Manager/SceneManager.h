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

	// ディレクショナルライトの方向
	//static constexpr VECTOR LIGHT_DIRECTION = { 0.3f, -0.7f, 0.8f };

	// シーン管理用
	enum class SCENE_ID
	{
		NONE,
		TITLE,					// タイトル
		PLAYERNUMBERSELECT,		// プレイヤー数選択
		EXAMPLE,				// 説明
		BUYSELECT,				// アイテム選択・購入
		GAME,					// ゲームシーン
		RESULT,					// 結果
	};

	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static SceneManager& GetInstance(void);

	// 初期化
	void Init(void);

	// 3Dの初期化
	void Init3D(void);

	// 更新
	void Update(void);

	// 描画
	void Draw(void);

	// リソースの破棄
	void Destroy(void);

	// 状態遷移
	void ChangeScene(SCENE_ID nextId);

	// シーンIDの取得
	SCENE_ID GetSceneID(void) const {return sceneId_;}

	// デルタタイムの取得
	float GetDeltaTime(void) const {return deltaTime_;}

	// カメラの取得
	Camera* GetCamera(void) const {return camera_;}

private:

	// 静的インスタンス
	static SceneManager* instance_;

	SCENE_ID sceneId_;
	SCENE_ID waitSceneId_;

	// フェード
	Fader* fader_;

	// 各種シーン
	SceneBase* scene_;

	// カメラ
	Camera* camera_;

	// シーン遷移中判定
	bool isSceneChanging_;

	//ここから下にobjectのポインタを追加していく


	// デルタタイム
	std::chrono::system_clock::time_point preTime_;
	float deltaTime_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	SceneManager(void);

	// コピーコンストラクタも同様
	SceneManager(const SceneManager& instance) = default;

	// デストラクタも同様
	~SceneManager(void) = default;

	// デルタタイムをリセットする
	void ResetDeltaTime(void);

	// シーン遷移
	void DoChangeScene(SCENE_ID sceneId);

	// フェード
	void Fade(void);

	// プレイヤー数
	int playerNum_;

	// キャラ選択で決まった各プレイヤー数をセット・ゲットする
	std::vector<int> selectedPlayerNums_;

	public:
		void SetPlayerNum(int num) { playerNum_ = num; }
		int GetPlayerNum(void) const { return playerNum_; }

		// 選択タイプの保存と取得
		void SetSelectedPlayerNums(const std::vector<int>& types) { selectedPlayerNums_ = types; }
		const std::vector<int>& GetSelectedPlayerNums() const { return selectedPlayerNums_; }

};