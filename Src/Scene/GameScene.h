#pragma once
#include "SceneBase.h"
#include "../Object/Actor/Stage/Stage.h"
#include <vector>

class Stage;
class Player;
class Subject;
class SubjectManager;
class ColliderBase;

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
	static constexpr VECTOR PLAYER2_INIT_POS = { 200.0f, 1000.0f, 0.0f };
	static constexpr VECTOR PLAYER3_INIT_POS = { -200.0f, 1000.0f, 0.0f };
	static constexpr VECTOR PLAYER4_INIT_POS = { 700.0f, 1000.0f, 0.0f };

	static constexpr VECTOR SUBJECT_AREA_MIN = { -600.0f, 0.0f, -600.0f };
	static constexpr VECTOR SUBJECT_AREA_MAX = { 600.0f, 0.0f, 600.0f };
	static constexpr int SUBJECT_COUNT = 6;

	static constexpr int PHOTO_SCORE_MAX = 1000;
	static constexpr int PHOTO_SCORE_MIN = 100;
	static constexpr float PHOTO_SCORE_NEAR_DISTANCE = 100.0f;
	static constexpr float PHOTO_SCORE_FAR_DISTANCE = 1200.0f;
	static constexpr float PHOTO_SCORE_VIEW_DOT_MIN = 0.7f;

	static constexpr int FLASH_FRAME_MAX = 12;
	static constexpr int THUMBNAIL_WIDTH = 320;
	static constexpr int THUMBNAIL_HEIGHT = 180;
	static constexpr int THUMBNAIL_MARGIN = 20;
	static constexpr int THUMBNAIL_FRAME_THICKNESS = 3;
	static constexpr int THUMBNAIL_LABEL_HEIGHT = 28;

	static constexpr float CAMERA_OCCLUDED_OPACITY = 0.25f;
	static constexpr float CAMERA_OCCLUDE_EPSILON = 1.0f;

	static constexpr int PREVIEW_WIDTH = 640;
	static constexpr int PREVIEW_HEIGHT = 360;

	static constexpr VECTOR GOAL_POS = { 520.0f, 0.0f, 520.0f };
	static constexpr float GOAL_RADIUS = 80.0f;

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
	void CaptureScreenshot(void);
	void DrawScreenshotThumbnail(void) const;
	void DrawFlashEffect(void) const;
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
	void ApplyPhotoScoreResult(int totalAddedScore);

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
	};

	// トラップ設定
	static constexpr int SPIKE_DURATION_FRAMES = 4 * 60; // 4秒
	static constexpr float SPIKE_TRIGGER_RADIUS = 40.0f;
	static constexpr float MINE_TRIGGER_RADIUS = 40.0f;
	static constexpr float MINE_DAMAGE_RADIUS = 120.0f;

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

	// 追加
	std::vector<Trap> traps_;

	// アイコンハンドル（ヘルメット・フラグ・スパイク・地雷）
	int iconHelmetHandle_ = -1;
	int iconFragHandle_ = -1;
	int iconSpikeHandle_ = -1;
	int iconMineHandle_ = -1;
};