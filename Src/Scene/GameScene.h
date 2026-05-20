#pragma once
#include "SceneBase.h"
#include "../Object/Actor/Stage/Stage.h"

class Stage;
class Player;
class Subject;
class SubjectManager;

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

	static constexpr VECTOR GOAL_POS = { 520.0f, 0.0f, 520.0f };
	static constexpr float GOAL_RADIUS = 80.0f;

	void DrawView(
		int screenHandle,
		int drawWidth,
		int drawHeight,
		const Player* targetPlayer,
		const Player* hidePlayer,
		const char* playerName);

	void DrawCompositedScene(void);
	void CaptureScreenshot(void);
	void DrawScreenshotThumbnail(void) const;
	void DrawFlashEffect(void) const;
	void DrawGoalMarker(void) const;
	void DrawSubjectDistanceGuide(const Player* targetPlayer) const;

	bool IsCameraOccludedByStage(const Player* targetPlayer) const;
	void ApplyStageOpacityForCamera(const Player* targetPlayer);

	void UpdateSubjectAttacks(void);
	bool IsPlayerReachedGoal(void) const;

	void TryTakePhoto(void);
	bool IsSubjectInView(const Player* targetPlayer, const Subject* targetSubject) const;
	bool IsSubjectVisible(const Player* targetPlayer, const Subject* targetSubject) const;
	int CalculatePhotoScore(const VECTOR& shotPos, const VECTOR& targetPos) const;

	Stage* stage_;
	Player* player_;
	Player* player2_;
	SubjectManager* subjectManager_;
	int leftScreenHandle_;
	int rightScreenHandle_;
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
};
