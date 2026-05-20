#include <DxLib.h>
#include "GameScene.h"
#include "../Object/Actor/Stage/Stage.h"
#include "../Object/Actor/Charactor/Player/Player.h"
#include "../Object/Actor/Charactor/Subject/Subject.h"
#include "../Object/Collider/ColliderModel.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/SubjectManager.h"

GameScene::GameScene()
	:
	stage_(nullptr),
	player_(nullptr),
	player2_(nullptr),
	subjectManager_(nullptr),
	leftScreenHandle_(-1),
	rightScreenHandle_(-1),
	sceneScreenHandle_(-1),
	screenshotScreenHandle_(-1),
	screenWidth_(0),
	screenHeight_(0),
	isSplitScreenEnabled_(true),
	isScreenshotRequested_(false),
	hasScreenshot_(false),
	isScreenshotPreviewEnabled_(false),
	flashFrame_(0),
	lastPhotoScore_(0),
	photoCount_(0)
{
}

GameScene::~GameScene()
{
}

void GameScene::Init()
{
	SceneManager& scene = SceneManager::GetInstance();

	stage_ = new Stage();
	stage_->Init();

	const ColliderBase* stageCollider =
		stage_->GetOwnCollider(static_cast<int>(Stage::COLLIDER_TYPE::MODEL));

	player_ = new Player();
	player_->Init();
	player_->AddHitCollider(stageCollider);

	player2_ = new Player();
	player2_->Init();
	player2_->SetInputEnabled(false);
	player2_->SetPos(PLAYER2_INIT_POS);
	player2_->SetCameraAngles(VGet(0.0f, DX_PI_F, 0.0f));
	player2_->AddHitCollider(stageCollider);

	subjectManager_ = new SubjectManager();
	subjectManager_->Init();
	subjectManager_->AddHitCollider(stageCollider);
	subjectManager_->SetMoveArea(SUBJECT_AREA_MIN, SUBJECT_AREA_MAX);

	for (int i = 0; i < SUBJECT_COUNT; i++)
	{
		subjectManager_->CreateRandomSubject(ResourceManager::SRC::SUBJECT);
	}

	isSplitScreenEnabled_ = scene.IsSplitScreenEnabled();

	GetDrawScreenSize(&screenWidth_, &screenHeight_);

	if (isSplitScreenEnabled_)
	{
		leftScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_, TRUE);
		rightScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_, TRUE);
	}

	sceneScreenHandle_ = MakeScreen(screenWidth_, screenHeight_, TRUE);
	screenshotScreenHandle_ = MakeScreen(screenWidth_, screenHeight_, TRUE);

	lastPhotoScore_ = 0;
	photoCount_ = 0;
	isScreenshotRequested_ = false;
	hasScreenshot_ = false;
	isScreenshotPreviewEnabled_ = false;
	flashFrame_ = 0;

	scene.SetGameResult(SceneManager::GAME_RESULT::NONE);
	scene.SetPhotoCount(0);
	scene.SetLastPhotoScore(0);

	auto* camera = scene.GetCamera();
	camera->SetAngles(player_->GetCameraAngles());
	camera->ChangeMode(Camera::MODE::FREE);
}

void GameScene::Update()
{
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	if (flashFrame_ > 0)
	{
		flashFrame_--;
	}

	stage_->Update();
	player_->Update();
	player2_->Update();

	if (subjectManager_ != nullptr)
	{
		subjectManager_->Update();
	}

	UpdateSubjectAttacks();

	if (IsPlayerReachedGoal())
	{
		scene.SetGameResult(SceneManager::GAME_RESULT::CLEAR);
		scene.SetPhotoCount(photoCount_);
		scene.SetLastPhotoScore(lastPhotoScore_);
		scene.ChangeScene(SceneManager::SCENE_ID::RESULT);
		return;
	}

	if (player_ != nullptr && player_->IsDead())
	{
		scene.SetGameResult(SceneManager::GAME_RESULT::GAMEOVER);
		scene.SetPhotoCount(photoCount_);
		scene.SetLastPhotoScore(lastPhotoScore_);
		scene.ChangeScene(SceneManager::SCENE_ID::RESULT);
		return;
	}

	if (ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		TryTakePhoto();
		isScreenshotRequested_ = true;
		flashFrame_ = FLASH_FRAME_MAX;
	}

	if (ins.IsTrgDown(KEY_INPUT_F1) && hasScreenshot_)
	{
		isScreenshotPreviewEnabled_ = !isScreenshotPreviewEnabled_;
	}
}

void GameScene::Draw()
{
	DrawCompositedScene();

	if (isScreenshotRequested_)
	{
		CaptureScreenshot();
	}

	SetDrawScreen(DX_SCREEN_BACK);
	SetDrawArea(0, 0, screenWidth_, screenHeight_);
	ClearDrawScreen();

	if (isScreenshotPreviewEnabled_ && hasScreenshot_)
	{
		DrawGraph(0, 0, screenshotScreenHandle_, FALSE);
		DrawString(20, 140, "F1 : CLOSE SCREENSHOT", GetColor(255, 255, 255));
	}
	else
	{
		DrawGraph(0, 0, sceneScreenHandle_, FALSE);

		if (hasScreenshot_)
		{
			DrawScreenshotThumbnail();
			DrawString(20, 140, "F1 : OPEN SCREENSHOT", GetColor(255, 255, 255));
		}
	}

	DrawFlashEffect();
}

void GameScene::Draw3D()
{
	// 3D•`‰æ‚ª•K—v‚Èê‡‚Í‚±‚±‚É’Ç‰Á
}

void GameScene::Release()
{
	if (player_)
	{
		player_->Release();
		delete player_;
		player_ = nullptr;
	}

	if (player2_)
	{
		player2_->Release();
		delete player2_;
		player2_ = nullptr;
	}

	if (subjectManager_)
	{
		subjectManager_->Release();
		delete subjectManager_;
		subjectManager_ = nullptr;
	}

	if (stage_)
	{
		stage_->Release();
		delete stage_;
		stage_ = nullptr;
	}

	if (leftScreenHandle_ != -1)
	{
		DeleteGraph(leftScreenHandle_);
		leftScreenHandle_ = -1;
	}

	if (rightScreenHandle_ != -1)
	{
		DeleteGraph(rightScreenHandle_);
		rightScreenHandle_ = -1;
	}

	if (sceneScreenHandle_ != -1)
	{
		DeleteGraph(sceneScreenHandle_);
		sceneScreenHandle_ = -1;
	}

	if (screenshotScreenHandle_ != -1)
	{
		DeleteGraph(screenshotScreenHandle_);
		screenshotScreenHandle_ = -1;
	}
}

void GameScene::DrawView(
	int screenHandle,
	int drawWidth,
	int drawHeight,
	const Player* targetPlayer,
	const Player* hidePlayer,
	const char* playerName)
{
	if (screenHandle == -1 || targetPlayer == nullptr)
	{
		return;
	}

	auto* camera = SceneManager::GetInstance().GetCamera();

	SetDrawScreen(screenHandle);
	SetDrawArea(0, 0, drawWidth, drawHeight);
	ClearDrawScreen();

	camera->SetPos(targetPlayer->GetCameraWorldPos());
	camera->SetAngles(targetPlayer->GetCameraAngles());
	camera->SetBeforeDraw();

	ApplyStageOpacityForCamera(targetPlayer);
	stage_->Draw();
	stage_->SetOpacityRate(1.0f);

	DrawGoalMarker();

	if (subjectManager_ != nullptr)
	{
		subjectManager_->Draw();
	}

	DrawSubjectDistanceGuide(targetPlayer);

	if (player_ != hidePlayer)
	{
		player_->Draw();
	}
	if (player2_ != hidePlayer)
	{
		player2_->Draw();
	}

	DrawString(20, 20, playerName, GetColor(255, 255, 255));
	DrawFormatString(20, 50, GetColor(255, 255, 0), "SCORE : %d", SceneManager::GetInstance().GetCarryMoney());
	DrawFormatString(20, 80, GetColor(0, 255, 255), "LAST PHOTO : +%d", lastPhotoScore_);
	DrawFormatString(20, 110, GetColor(255, 255, 255), "PHOTO COUNT : %d", photoCount_);

	if (targetPlayer == player_)
	{
		const int barX = 20;
		const int barY = 145;
		const int barWidth = drawWidth >= 900 ? 240 : 180;
		const int barHeight = 18;
		const int backColor = GetColor(40, 40, 40);
		const int frameColor = GetColor(255, 255, 255);
		const int hpColor = GetColor(80, 220, 80);
		const int damageColor = GetColor(255, 90, 90);
		const int fillWidth = static_cast<int>(barWidth * player_->GetHpRate());
		const int currentColor = player_->CanTakeDamage() ? hpColor : damageColor;

		DrawString(barX, barY, "HP", GetColor(255, 255, 255));
		DrawBox(barX, barY + 22, barX + barWidth, barY + 22 + barHeight, backColor, TRUE);

		if (fillWidth > 0)
		{
			DrawBox(barX, barY + 22, barX + fillWidth, barY + 22 + barHeight, currentColor, TRUE);
		}

		DrawBox(barX, barY + 22, barX + barWidth, barY + 22 + barHeight, frameColor, FALSE);
		DrawFormatString(
			barX + barWidth + 12,
			barY + 22,
			GetColor(255, 255, 255),
			"%d / %d",
			player_->GetHp(),
			player_->GetHpMax());
	}
}

void GameScene::DrawCompositedScene(void)
{
	if (sceneScreenHandle_ == -1)
	{
		return;
	}

	if (!isSplitScreenEnabled_)
	{
		DrawView(
			sceneScreenHandle_,
			screenWidth_,
			screenHeight_,
			player_,
			nullptr,
			"PLAYER 1");
		return;
	}

	DrawView(
		leftScreenHandle_,
		screenWidth_ / 2,
		screenHeight_,
		player_,
		nullptr,
		"PLAYER 1");

	DrawView(
		rightScreenHandle_,
		screenWidth_ / 2,
		screenHeight_,
		player2_,
		nullptr,
		"PLAYER 2");

	SetDrawScreen(sceneScreenHandle_);
	SetDrawArea(0, 0, screenWidth_, screenHeight_);
	ClearDrawScreen();

	DrawGraph(0, 0, leftScreenHandle_, FALSE);
	DrawGraph(screenWidth_ / 2, 0, rightScreenHandle_, FALSE);
	DrawBox(
		screenWidth_ / 2 - 1,
		0,
		screenWidth_ / 2 + 1,
		screenHeight_,
		GetColor(255, 255, 255),
		TRUE);
}

void GameScene::CaptureScreenshot(void)
{
	if (sceneScreenHandle_ == -1 || screenshotScreenHandle_ == -1)
	{
		isScreenshotRequested_ = false;
		return;
	}

	SetDrawScreen(screenshotScreenHandle_);
	SetDrawArea(0, 0, screenWidth_, screenHeight_);
	ClearDrawScreen();
	DrawGraph(0, 0, sceneScreenHandle_, FALSE);

	hasScreenshot_ = true;
	isScreenshotRequested_ = false;
}

void GameScene::DrawScreenshotThumbnail(void) const
{
	if (!hasScreenshot_ || screenshotScreenHandle_ == -1)
	{
		return;
	}

	const int thumbnailRight = screenWidth_ - THUMBNAIL_MARGIN;
	const int thumbnailLeft = thumbnailRight - THUMBNAIL_WIDTH;
	const int thumbnailTop = THUMBNAIL_MARGIN + THUMBNAIL_LABEL_HEIGHT;
	const int thumbnailBottom = thumbnailTop + THUMBNAIL_HEIGHT;
	const int frameColor = GetColor(255, 255, 255);
	const int backColor = GetColor(0, 0, 0);
	const int labelColor = GetColor(255, 255, 0);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	DrawBox(
		thumbnailLeft - THUMBNAIL_FRAME_THICKNESS,
		THUMBNAIL_MARGIN - THUMBNAIL_FRAME_THICKNESS,
		thumbnailRight + THUMBNAIL_FRAME_THICKNESS,
		thumbnailBottom + THUMBNAIL_FRAME_THICKNESS,
		backColor,
		TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	DrawBox(
		thumbnailLeft - THUMBNAIL_FRAME_THICKNESS,
		thumbnailTop - THUMBNAIL_FRAME_THICKNESS,
		thumbnailRight + THUMBNAIL_FRAME_THICKNESS,
		thumbnailBottom + THUMBNAIL_FRAME_THICKNESS,
		frameColor,
		TRUE);

	DrawExtendGraph(
		thumbnailLeft,
		thumbnailTop,
		thumbnailRight,
		thumbnailBottom,
		screenshotScreenHandle_,
		FALSE);

	DrawString(thumbnailLeft, THUMBNAIL_MARGIN, "LAST SHOT", labelColor);
}

void GameScene::DrawFlashEffect(void) const
{
	if (flashFrame_ <= 0)
	{
		return;
	}

	const int alpha = 255 * flashFrame_ / FLASH_FRAME_MAX;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, screenWidth_, screenHeight_, GetColor(255, 255, 255), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

bool GameScene::IsSubjectInView(const Player* targetPlayer, const Subject* targetSubject) const
{
	if (targetPlayer == nullptr || targetSubject == nullptr)
	{
		return false;
	}

	const VECTOR cameraPos = targetPlayer->GetCameraWorldPos();
	const VECTOR subjectHeadPos = VAdd(
		targetSubject->GetTransform().pos,
		Subject::COL_CAPSULE_TOP_LOCAL_POS);

	const VECTOR toSubject = VSub(subjectHeadPos, cameraPos);
	const float distance = VSize(toSubject);

	if (distance <= 0.0001f)
	{
		return true;
	}

	const VECTOR subjectDir = VScale(toSubject, 1.0f / distance);
	const VECTOR cameraForward = targetPlayer->GetCameraForward();

	const float dot =
		cameraForward.x * subjectDir.x +
		cameraForward.y * subjectDir.y +
		cameraForward.z * subjectDir.z;

	if (dot < PHOTO_SCORE_VIEW_DOT_MIN)
	{
		return false;
	}

	return IsSubjectVisible(targetPlayer, targetSubject);
}

int GameScene::CalculatePhotoScore(const VECTOR& shotPos, const VECTOR& targetPos) const
{
	const float distance = VSize(VSub(targetPos, shotPos));

	if (distance <= PHOTO_SCORE_NEAR_DISTANCE)
	{
		return PHOTO_SCORE_MAX;
	}

	if (distance >= PHOTO_SCORE_FAR_DISTANCE)
	{
		return PHOTO_SCORE_MIN;
	}

	const float t =
		(distance - PHOTO_SCORE_NEAR_DISTANCE) /
		(PHOTO_SCORE_FAR_DISTANCE - PHOTO_SCORE_NEAR_DISTANCE);

	int score = static_cast<int>(
		PHOTO_SCORE_MAX - (PHOTO_SCORE_MAX - PHOTO_SCORE_MIN) * t);

	if (score < PHOTO_SCORE_MIN)
	{
		score = PHOTO_SCORE_MIN;
	}
	if (score > PHOTO_SCORE_MAX)
	{
		score = PHOTO_SCORE_MAX;
	}

	return score;
}

void GameScene::TryTakePhoto(void)
{
	if (player_ == nullptr || subjectManager_ == nullptr)
	{
		return;
	}

	const auto& subjects = subjectManager_->GetSubjects();
	if (subjects.empty())
	{
		return;
	}

	photoCount_++;

	SceneManager& scene = SceneManager::GetInstance();
	const VECTOR shotPos = player_->GetCameraWorldPos();

	int addScore = 0;

	for (const auto* subject : subjects)
	{
		if (subject == nullptr)
		{
			continue;
		}

		if (!IsSubjectInView(player_, subject))
		{
			continue;
		}

		addScore += CalculatePhotoScore(shotPos, subject->GetTransform().pos);
	}

	lastPhotoScore_ = addScore;
	scene.SetCarryMoney(scene.GetCarryMoney() + addScore);
}

void GameScene::DrawSubjectDistanceGuide(const Player* targetPlayer) const
{
	if (targetPlayer == nullptr || subjectManager_ == nullptr)
	{
		return;
	}

	const auto& subjects = subjectManager_->GetSubjects();
	if (subjects.empty())
	{
		return;
	}

	const VECTOR playerHeadPos = VAdd(
		targetPlayer->GetTransform().pos,
		Player::COL_CAPSULE_TOP_LOCAL_POS);

	const int visibleLineColor = GetColor(255, 0, 0);
	const int hiddenLineColor = GetColor(0, 0, 255);
	const int textColor = GetColor(255, 255, 0);

	for (const auto* subject : subjects)
	{
		if (subject == nullptr)
		{
			continue;
		}

		const VECTOR subjectHeadPos = VAdd(
			subject->GetTransform().pos,
			Subject::COL_CAPSULE_TOP_LOCAL_POS);

		const float distance = VSize(VSub(subjectHeadPos, playerHeadPos));
		const bool isVisible = IsSubjectVisible(targetPlayer, subject);
		const int lineColor = isVisible ? visibleLineColor : hiddenLineColor;

		DrawLine3D(playerHeadPos, subjectHeadPos, lineColor);

		const VECTOR midPos = VScale(VAdd(playerHeadPos, subjectHeadPos), 0.5f);
		const VECTOR screenPos = ConvWorldPosToScreenPos(midPos);

		DrawFormatString(
			static_cast<int>(screenPos.x),
			static_cast<int>(screenPos.y),
			textColor,
			"%.0f",
			distance);
	}
}

bool GameScene::IsSubjectVisible(const Player* targetPlayer, const Subject* targetSubject) const
{
	if (targetPlayer == nullptr || targetSubject == nullptr || stage_ == nullptr)
	{
		return false;
	}

	const ColliderBase* stageColliderBase =
		stage_->GetOwnCollider(static_cast<int>(Stage::COLLIDER_TYPE::MODEL));

	if (stageColliderBase == nullptr ||
		stageColliderBase->GetShape() != ColliderBase::SHAPE::MODEL)
	{
		return true;
	}

	const auto* stageCollider = static_cast<const ColliderModel*>(stageColliderBase);

	const VECTOR cameraPos = targetPlayer->GetCameraWorldPos();
	const VECTOR subjectHeadPos = VAdd(
		targetSubject->GetTransform().pos,
		Subject::COL_CAPSULE_TOP_LOCAL_POS);

	auto hit = stageCollider->GetNearestHitPolyLine(cameraPos, subjectHeadPos, true);

	if (!hit.HitFlag)
	{
		return true;
	}

	const float hitDistance = VSize(VSub(hit.HitPosition, cameraPos));
	const float subjectDistance = VSize(VSub(subjectHeadPos, cameraPos));

	return hitDistance >= subjectDistance - 1.0f;
}

bool GameScene::IsCameraOccludedByStage(const Player* targetPlayer) const
{
	if (targetPlayer == nullptr || stage_ == nullptr)
	{
		return false;
	}

	const ColliderBase* stageColliderBase =
		stage_->GetOwnCollider(static_cast<int>(Stage::COLLIDER_TYPE::MODEL));

	if (stageColliderBase == nullptr ||
		stageColliderBase->GetShape() != ColliderBase::SHAPE::MODEL)
	{
		return false;
	}

	const auto* stageCollider = static_cast<const ColliderModel*>(stageColliderBase);

	const VECTOR focusPos = VAdd(
		targetPlayer->GetTransform().pos,
		Player::COL_CAPSULE_TOP_LOCAL_POS);

	const VECTOR cameraPos = targetPlayer->GetCameraWorldPos();

	auto hit = stageCollider->GetNearestHitPolyLine(focusPos, cameraPos, true);

	if (!hit.HitFlag)
	{
		return false;
	}

	const float hitDistance = VSize(VSub(hit.HitPosition, focusPos));
	const float cameraDistance = VSize(VSub(cameraPos, focusPos));

	return hitDistance < cameraDistance - CAMERA_OCCLUDE_EPSILON;
}

void GameScene::ApplyStageOpacityForCamera(const Player* targetPlayer)
{
	if (stage_ == nullptr)
	{
		return;
	}

	if (IsCameraOccludedByStage(targetPlayer))
	{
		stage_->SetOpacityRate(CAMERA_OCCLUDED_OPACITY);
	}
	else
	{
		stage_->SetOpacityRate(1.0f);
	}
}

void GameScene::UpdateSubjectAttacks(void)
{
	if (player_ == nullptr || subjectManager_ == nullptr)
	{
		return;
	}

	const VECTOR playerPos = player_->GetTransform().pos;
	const auto& subjects = subjectManager_->GetSubjects();

	for (auto* subject : subjects)
	{
		if (subject == nullptr)
		{
			continue;
		}

		if (subject->CanStartAttack() && subject->IsInAttackRange(playerPos))
		{
			subject->StartAttack(playerPos);
		}

		if (subject->ConsumeAttackHit() &&
			subject->IsInAttackRange(playerPos))
		{
			player_->TakeDamage(1);
		}
	}
}

bool GameScene::IsPlayerReachedGoal(void) const
{
	if (player_ == nullptr)
	{
		return false;
	}

	VECTOR diff = VSub(player_->GetTransform().pos, GOAL_POS);
	diff.y = 0.0f;

	return VSize(diff) <= GOAL_RADIUS;
}

void GameScene::DrawGoalMarker(void) const
{
	const VECTOR spherePos = VAdd(GOAL_POS, VGet(0.0f, 45.0f, 0.0f));
	const VECTOR poleTop = VAdd(GOAL_POS, VGet(0.0f, 180.0f, 0.0f));
	const int ringColor = GetColor(0, 255, 120);

	DrawSphere3D(spherePos, GOAL_RADIUS, 16, ringColor, ringColor, FALSE);
	DrawLine3D(GOAL_POS, poleTop, ringColor);
}