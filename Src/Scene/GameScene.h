#pragma once
#include "SceneBase.h"
#include "../Object/Actor/Stage/Stage.h"
#include <vector>
#include <string>

class Stage;
class Player;
class Subject;
class SubjectManager;
class ColliderBase;
class EffectManager;

class GameScene : public SceneBase
{
public:
	GameScene();
	~GameScene(void) override;

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Draw3D(void);
	void Release(void) override;

private:

	// 定数
	// プレイヤー初期位置
	static constexpr VECTOR PLAYER2_INIT_POS = { 200.0f, 1000.0f, 0.0f };
	static constexpr VECTOR PLAYER3_INIT_POS = { -200.0f, 1000.0f, 0.0f };
	static constexpr VECTOR PLAYER4_INIT_POS = { 700.0f, 1000.0f, 0.0f };
	// 撮影対象の出現範囲
	static constexpr VECTOR SUBJECT_AREA_MIN = { -3600.0f, 0.0f, -790.0f };
	static constexpr VECTOR SUBJECT_AREA_MAX = { 11100.0f, 0.0f, 11900.0f };
	static constexpr int SUBJECT_COUNT = 50;
	// 撮影スコア関連
	static constexpr int PHOTO_SCORE_MAX = 1000;
	static constexpr int PHOTO_SCORE_MIN = 0;
	static constexpr float PHOTO_SCORE_NEAR_DISTANCE = 100.0f;        // 近距離最大スコア閾値（変更なし）
	static constexpr float PHOTO_SCORE_FAR_DISTANCE = 750.0f;        // 遠距離での最小スコア判定距
	static constexpr float PHOTO_SCORE_VIEW_DOT_MIN = 0.70f;         // 視野角判定を厳しく（dot閾値を大きく）
	// フラッシュエフェクト関連
	static constexpr int FLASH_FRAME_MAX = 12;
	static constexpr int THUMBNAIL_WIDTH = 320;
	static constexpr int THUMBNAIL_HEIGHT = 180;
	static constexpr int THUMBNAIL_MARGIN = 20;
	static constexpr int THUMBNAIL_FRAME_THICKNESS = 3;
	static constexpr int THUMBNAIL_LABEL_HEIGHT = 28;
	// カメラがステージに隠れているときのステージの不透明度と、隠れているかどうかの判定に使う距離の閾値
	static constexpr float CAMERA_OCCLUDED_OPACITY = 0.25f;
	static constexpr float CAMERA_OCCLUDE_EPSILON = 1.0f;

	static constexpr int PREVIEW_WIDTH = 640;
	static constexpr int PREVIEW_HEIGHT = 360;

	static constexpr int MAX_PHOTO_COUNT = 50;

	/*static constexpr VECTOR GOAL_POS = { 520.0f, 0.0f, 520.0f };
	static constexpr float GOAL_RADIUS = 80.0f;*/

	static constexpr VECTOR GOAL_CANDIDATES[5] =
	{
		{ 2280.0f, 0.0f, 1300.0f },
		{ 1520.0f, 0.0f, 520.0f },
		{ 4020.0f, 0.0f, 620.0f },
		{ 2020.0f, 0.0f, 1120.0f },
		{ 1520.0f, 0.0f, 1320.0f }
	};

	static constexpr float GOAL_RADIUS = 80.0f;
	static constexpr int GOAL_CANDIDATE_COUNT = 5;

	void DrawView(
		int screenHandle,
		int drawWidth,
		int drawHeight,
		const Player* targetPlayer,
		const Player* hidePlayer,
		const char* playerName);

	void DrawDeadView(
		int screenHandle,
		int drawWidth,
		int drawHeight,
		const char* playerName) const;

	void DrawCompositedScene(void);
	void CaptureScreenshot(int playerIndex);
	void DrawScreenshotThumbnail(void) const;
	void DrawPhotoCards(int playerIndex);
	void DrawFlashEffect(int playerIndex);
	void DrawShutterEffect(int playerIndex);
	void DrawPlayerScreen(int playerIndex);
	void DrawRankEffect(int playerIndex);
	void DrawSubjectDistanceGuide(const Player* targetPlayer) const;

	bool IsCameraOccludedByStage(const Player* targetPlayer) const;
	void ApplyStageOpacityForCamera(const Player* targetPlayer);

	void UpdateSubjectAttacks(void);
	bool IsPlayerAlive(const Player* targetPlayer) const;
	bool IsPlayerAtGoal(const Player* targetPlayer) const;
	bool IsPlayerReachedGoal(void) const;
	bool IsAllPlayersDead(void) const;

	void TryTakePhoto(void);
	bool IsSubjectInView(const Player* targetPlayer, const Subject* targetSubject) const;
	bool IsSubjectVisible(const Player* targetPlayer, const Subject* targetSubject) const;
	int CalculatePhotoScore(const VECTOR& shotPos, const VECTOR& targetPos) const;
	int CalculatePlayerPhotoScore(const Player* targetPlayer) const;
	void ApplyPhotoScoreResult(
		int playerIndex,
		int addedScore);

	Player* CreatePlayer(
		const ColliderBase* stageCollider,
		const VECTOR* initPos = nullptr,
		bool usePlayer2InputConfig = false,
		bool enableInput = false);

	void RebuildPlayersArray(void);
	void UpdatePlayers(void);
	void ReleasePlayers(void);
	void DeleteScreenHandle(int& screenHandle);

	const Player* GetPlayerByIndex(int index) const;
	void DrawPlayers(const Player* hidePlayer);

	void SetupPlayers(const ColliderBase* stageCollider, int selectedPlayerCount);
	void ResetPlayerSlots(void);
	void CreateScreenHandles(int selectedPlayerCount);
	void ReleaseScreenHandles(void);
	void ResetScreenHandles(void);

	void DrawSinglePlayerScene(void);
	void DrawTwoPlayerScene(void);
	void DrawFourPlayerScene(void);
	void ComposeSplitScreens(bool isFourWay);
	void DrawEmptyView(int screenHandle, int drawWidth, int drawHeight) const;

	void DrawViewWorld(const Player* targetPlayer, const Player* hidePlayer);
	void DrawViewHud(const Player* targetPlayer, const char* playerName, int drawWidth) const;
	void DrawPlayerHpBar(const Player* targetPlayer, int drawWidth) const;
	void DrawPlayerPhotoInfo(const Player* targetPlayer) const;

	void DrawScreenshotPreview(void) const;
	void GetPlayer1ViewArea(int& x, int& y, int& width, int& height) const;

	// Inventory HUD
	void DrawInventoryHUD(const Player* targetPlayer, int drawWidth, int drawHeight) const;

	// 手榴弾関連
	void ExplodeGrenade(const VECTOR& pos);

	static constexpr int ITEM_ICON_SIZE = 48;
	static constexpr int ITEM_ICON_SPACING = 8;
	static constexpr int ITEM_ICON_MARGIN = 16;

	// 追加: トラップ関連
	enum class TRAP_TYPE { SPIKE = 0, MINE = 1 };
	struct Trap
	{
		TRAP_TYPE type;
		VECTOR pos;
		bool triggered = false;
		int lifeFrames = 0; // 残存フレーム（スパイク持続等）
		int ownerPlayerIndex = 0; // owner index in players_ (optional)

		int modelId = -1;
	};

	//写真評価演出関連
	struct PhotoEffect
	{
		int flashFrame = 0;
		int shutterFrame = 0;
		int rankFrame = 0;
		int flashDelay = 0;

		std::string rankText;
		int rankColor = GetColor(255, 255, 255);

		int cooldown = 0;
		int remainingPhoto = MAX_PHOTO_COUNT;
	};
	// 写真カードの描画情報
	struct PhotoCard
	{
		bool active = false;

		float x;
		float y;

		float targetX;
		float targetY;

		float scale = 1.0f;

		float angle = 0.0f;       // 現在角度
		float targetAngle = 0.0f; // 最終角度

		int frame = 0;

		int alpha = 255;      
		bool fading = false;  

		int graph = -1;
		int playerIndex = 0;
		int polaroidHandle = -1;

		int score = 0;
	};
	std::vector<PhotoCard> photoCards_;
	std::vector<PhotoEffect> photoEffects_;
	// トラップ設定
	static constexpr int SPIKE_DURATION_FRAMES = 4 * 60; // 4秒
	static constexpr float SPIKE_TRIGGER_RADIUS = 40.0f;
	static constexpr float MINE_TRIGGER_RADIUS = 40.0f;
	static constexpr float MINE_DAMAGE_RADIUS = 120.0f;

	//手榴弾関連
	struct Grenade
	{
		VECTOR pos;
		VECTOR velocity;

		bool exploded = false;
		int lifeFrame = 0;
	};



	Stage* stage_;
	Player* player_;
	Player* player2_;
	Player* player3_;
	Player* player4_;
	SubjectManager* subjectManager_;
	int leftScreenHandle_;
	int rightScreenHandle_;
	int bottomLeftScreenHandle_;
	int bottomRightScreenHandle_;
	int sceneScreenHandle_;
	int screenshotScreenHandle_;
	int screenWidth_;
	int screenHeight_;
	bool isSplitScreenEnabled_;
	bool isScreenshotRequested_;
	bool hasScreenshot_;
	bool isScreenshotPreviewEnabled_;
	int flashFrame_;
	int lastPhotoScore_;
	int photoCount_;
	int activePlayerCount_;
	std::vector<Player*> players_;
	std::vector<int> lastPhotoScorePerPlayer_;
	std::vector<int> photoCountPerPlayer_;

	// メンバ変数：トラップ、手榴弾,爆発エフェクト
	std::vector<Trap> traps_;
	std::vector<Grenade> grenades_;

	std::unique_ptr<EffectManager>effectManager_;

	// アイコンハンドル（ヘルメット・フラグ・スパイク・地雷）
	int iconHelmetHandle_ = -1;
	int iconFragHandle_ = -1;
	int iconSpikeHandle_ = -1;
	int iconMineHandle_ = -1;

	// 追加: ワールド全体をどれくらい暗くするか（0.0 = 無効, 1.0 = 真っ黒）
	float worldDarkness_;

	// ゴール
	VECTOR goalPos_;
	
		// 写真評価演出
	std::string photoRank_;
	int photoRankFrame_ = 0;
	int photoRankMaxFrame_ = 120;
	int photoRankFont_ = -1;
	int shutterFrame_ = 0;
	
	// シャッターの待ち時間
	static constexpr int PHOTO_COOLDOWN = 45; // 約0.75秒(60FPS)

	int photoCooldown_ = 0;
	int photoIdleFrame_ = 0;
	int lastPhotoPlayerIndex_ = 0;

	bool isCycleItem;
	bool isUseItem;
	
	int remainingPhotoCount_ = -1;
};