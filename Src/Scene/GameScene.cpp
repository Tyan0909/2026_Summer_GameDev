#include <DxLib.h>
#include <vector>
#include <algorithm> // std::find
#include <cfloat>    // FLT_MAX
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
	player3_(nullptr),
	player4_(nullptr),
	subjectManager_(nullptr),
	leftScreenHandle_(-1),
	rightScreenHandle_(-1),
	bottomLeftScreenHandle_(-1),
	bottomRightScreenHandle_(-1),
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
	photoCount_(0),
	activePlayerCount_(1)
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

	// プレイヤー選択情報を取得して GameScene の構成に反映
	const int selectedPlayerCount = scene.GetPlayerNum();
	const auto& selectedPlayers = scene.GetSelectedPlayerNums();
	activePlayerCount_ = selectedPlayerCount;

	// プレイヤー1（常に作成）
	player_ = new Player();
	player_->Init();
	player_->AddHitCollider(stageCollider);

	// プレイヤー2は選択人数が2人以上の場合のみ作成
	if (selectedPlayerCount >= 2)
	{
		player2_ = new Player();
		player2_->Init();
		player2_->SetInputConfig(Player::PLAYER2_KEYBOARD_INPUT_CONFIG);
		player2_->SetInputEnabled(true);
		player2_->SetPos(PLAYER2_INIT_POS);
		player2_->SetCameraAngles(VGet(0.0f, DX_PI_F, 0.0f));
		player2_->AddHitCollider(stageCollider);
	}
	else
	{
		player2_ = nullptr;
	}

	// プレイヤー3は選択人数が3人以上の場合のみ作成
	if (selectedPlayerCount >= 3)
	{
		player3_ = new Player();
		player3_->Init();
		player3_->SetInputEnabled(true);
		player3_->SetPos(PLAYER3_INIT_POS);
		player3_->SetCameraAngles(VGet(0.0f, DX_PI_F, 0.0f));
		player3_->AddHitCollider(stageCollider);
	}
	else
	{
		player3_ = nullptr;
	}

	// プレイヤー4は選択人数が4人の場合に作成（今は最大4に対応）
	if (selectedPlayerCount >= 4)
	{
		player4_ = new Player();
		player4_->Init();
		player4_->SetInputEnabled(true);
		player4_->SetPos(PLAYER4_INIT_POS);
		player4_->SetCameraAngles(VGet(0.0f, DX_PI_F, 0.0f));
		player4_->AddHitCollider(stageCollider);
	}
	else
	{
		player4_ = nullptr;
	}

	// --- players_ 配列を構築し、プレイヤーごとのスコア配列を初期化 ---
	players_.clear();
	if (player_) players_.push_back(player_);
	if (player2_) players_.push_back(player2_);
	if (player3_) players_.push_back(player3_);
	if (player4_) players_.push_back(player4_);

	const size_t pcount = players_.size();
	lastPhotoScorePerPlayer_.assign(pcount, 0);
	photoCountPerPlayer_.assign(pcount, 0);
	// -------------------------------------------------------------

	subjectManager_ = new SubjectManager();
	subjectManager_->Init();
	subjectManager_->AddHitCollider(stageCollider);
	subjectManager_->SetMoveArea(SUBJECT_AREA_MIN, SUBJECT_AREA_MAX);

	for (int i = 0; i < SUBJECT_COUNT; i++)
	{
		subjectManager_->CreateRandomSubject(ResourceManager::SRC::SUBJECT);
	}

	// 分割方法の決定:
	if (selectedPlayerCount <= 1)
	{
		isSplitScreenEnabled_ = false;
	}
	else
	{
		isSplitScreenEnabled_ = true;
	}

	GetDrawScreenSize(&screenWidth_, &screenHeight_);

	// 画面バッファ作成
	if (isSplitScreenEnabled_ && selectedPlayerCount == 2)
	{
		leftScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_, TRUE);
		rightScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_, TRUE);
		bottomLeftScreenHandle_ = -1;
		bottomRightScreenHandle_ = -1;
	}
	else if (isSplitScreenEnabled_ && selectedPlayerCount >= 3)
	{
		// 4分割 (各スクリーンは画面幅/2 x 高さ/2)
		leftScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_ / 2, TRUE); // top-left
		rightScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_ / 2, TRUE); // top-right
		bottomLeftScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_ / 2, TRUE); // bottom-left
		bottomRightScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_ / 2, TRUE); // bottom-right
	}
	else
	{
		leftScreenHandle_ = -1;
		rightScreenHandle_ = -1;
		bottomLeftScreenHandle_ = -1;
		bottomRightScreenHandle_ = -1;
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

	if (stage_) stage_->Update();
	if (player_) player_->Update();
	if (player2_) player2_->Update();
	if (player3_) player3_->Update();
	if (player4_) player4_->Update();

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

	if (IsAllPlayersDead())
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
	// 3D描画が必要な場合に処理を追加
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

	if (player3_)
	{
		player3_->Release();
		delete player3_;
		player3_ = nullptr;
	}

	if (player4_)
	{
		player4_->Release();
		delete player4_;
		player4_ = nullptr;
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

	if (bottomLeftScreenHandle_ != -1)
	{
		DeleteGraph(bottomLeftScreenHandle_);
		bottomLeftScreenHandle_ = -1;
	}

	if (bottomRightScreenHandle_ != -1)
	{
		DeleteGraph(bottomRightScreenHandle_);
		bottomRightScreenHandle_ = -1;
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

	if (!IsPlayerAlive(targetPlayer))
	{
		DrawDeadView(screenHandle, drawWidth, drawHeight, playerName);
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
	if (stage_) stage_->Draw();
	if (stage_) stage_->SetOpacityRate(1.0f);

	DrawGoalMarker();

	if (subjectManager_ != nullptr)
	{
		subjectManager_->Draw();
	}

	DrawSubjectDistanceGuide(targetPlayer);

	if (player_ != hidePlayer && player_) player_->Draw();
	if (player2_ != hidePlayer && player2_) player2_->Draw();
	if (player3_ != hidePlayer && player3_) player3_->Draw();
	if (player4_ != hidePlayer && player4_) player4_->Draw();

	DrawString(20, 20, playerName, GetColor(255, 255, 255));
	DrawFormatString(20, 50, GetColor(255, 255, 0), "SCORE : %d", SceneManager::GetInstance().GetCarryMoney());

	{
		const int barX = 20;
		const int barY = 145;
		const int barWidth = drawWidth >= 900 ? 240 : 180;
		const int barHeight = 18;
		const int backColor = GetColor(40, 40, 40);
		const int frameColor = GetColor(255, 255, 255);
		const int hpColor = GetColor(80, 220, 80);
		const int damageColor = GetColor(255, 90, 90);
		const int fillWidth = static_cast<int>(barWidth * targetPlayer->GetHpRate());
		const int currentColor = targetPlayer->CanTakeDamage() ? hpColor : damageColor;

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
			targetPlayer->GetHp(),
			targetPlayer->GetHpMax());
	}

	int localLast = lastPhotoScore_;
	int localCount = 0;

	auto it = std::find(players_.begin(), players_.end(), targetPlayer);
	if (it != players_.end())
	{
		const int idx = static_cast<int>(std::distance(players_.begin(), it));
		localLast = lastPhotoScorePerPlayer_[idx];
		localCount = photoCountPerPlayer_[idx];
	}
	else
	{
		localCount = photoCount_;
	}

	DrawFormatString(20, 80, GetColor(0, 255, 255), "LAST PHOTO : +%d", localLast);
	DrawFormatString(20, 110, GetColor(255, 255, 255), "PHOTO COUNT : %d", localCount);
}

void GameScene::DrawCompositedScene(void)
{
	if (sceneScreenHandle_ == -1)
	{
		return;
	}

	// 1人プレイ（フルスクリーン）
	if (!isSplitScreenEnabled_ || activePlayerCount_ <= 1)
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

	// 2人プレイ：左右分割
	if (activePlayerCount_ == 2)
	{
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
		return;
	}

	// 3人以上：2x2分割（3人は一枠暗くする）
	DrawView(
		leftScreenHandle_,
		screenWidth_ / 2,
		screenHeight_ / 2,
		player_,
		nullptr,
		"PLAYER 1");

	DrawView(
		rightScreenHandle_,
		screenWidth_ / 2,
		screenHeight_ / 2,
		player2_ ? player2_ : player_,
		nullptr,
		"PLAYER 2");

	DrawView(
		bottomLeftScreenHandle_,
		screenWidth_ / 2,
		screenHeight_ / 2,
		player3_ ? player3_ : player_,
		nullptr,
		"PLAYER 3");

	if (player4_)
	{
		DrawView(
			bottomRightScreenHandle_,
			screenWidth_ / 2,
			screenHeight_ / 2,
			player4_,
			nullptr,
			"PLAYER 4");
	}
	else
	{
		// 空き枠を暗くする
		SetDrawScreen(bottomRightScreenHandle_);
		SetDrawArea(0, 0, screenWidth_ / 2, screenHeight_ / 2);
		ClearDrawScreen();
		DrawBox(0, 0, screenWidth_ / 2, screenHeight_ / 2, GetColor(10, 10, 10), TRUE);
		DrawFormatString(screenWidth_ / 4 - 40, screenHeight_ / 4 - 8, GetColor(180, 180, 180), "NO PLAYER");
	}

	// 合成
	SetDrawScreen(sceneScreenHandle_);
	SetDrawArea(0, 0, screenWidth_, screenHeight_);
	ClearDrawScreen();

	DrawGraph(0, 0, leftScreenHandle_, FALSE);
	DrawGraph(screenWidth_ / 2, 0, rightScreenHandle_, FALSE);
	DrawGraph(0, screenHeight_ / 2, bottomLeftScreenHandle_, FALSE);
	DrawGraph(screenWidth_ / 2, screenHeight_ / 2, bottomRightScreenHandle_, FALSE);

	// 枠線
	DrawBox(
		screenWidth_ / 2 - 1,
		0,
		screenWidth_ / 2 + 1,
		screenHeight_,
		GetColor(255, 255, 255),
		TRUE);
	DrawBox(
		0,
		screenHeight_ / 2 - 1,
		screenWidth_,
		screenHeight_ / 2 + 1,
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
	if (players_.empty() || subjectManager_ == nullptr) return;

	const auto& subjects = subjectManager_->GetSubjects();
	if (subjects.empty()) return;

	int totalAddedScore = 0;

	for (size_t i = 0; i < players_.size(); ++i)
	{
		Player* p = players_[i];
		if (p == nullptr || !IsPlayerAlive(p)) continue;

		int addScore = 0;
		const VECTOR shotPos = p->GetCameraWorldPos();

		for (const auto* subject : subjects)
		{
			if (subject == nullptr) continue;
			if (!IsSubjectInView(p, subject)) continue;
			addScore += CalculatePhotoScore(shotPos, subject->GetTransform().pos);
		}

		lastPhotoScorePerPlayer_[i] = addScore;

		if (addScore > 0)
		{
			photoCountPerPlayer_[i] += 1;
			totalAddedScore += addScore;
		}
	}

	if (totalAddedScore > 0)
	{
		lastPhotoScore_ = totalAddedScore;
		photoCount_ = 0;
		for (int v : photoCountPerPlayer_)
		{
			photoCount_ += v;
		}

		SceneManager& scene = SceneManager::GetInstance();
		scene.SetCarryMoney(scene.GetCarryMoney() + totalAddedScore);
	}
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
	if (subjectManager_ == nullptr)
	{
		return;
	}

	const auto& subjects = subjectManager_->GetSubjects();
	if (subjects.empty()) return;

	// 集める
	std::vector<Player*> players;
	if (player_) players.push_back(player_);
	if (player2_) players.push_back(player2_);
	if (player3_) players.push_back(player3_);
	if (player4_) players.push_back(player4_);

	for (auto* subject : subjects)
	{
		if (subject == nullptr) continue;

		// 最も近いプレイヤーを見つける
		Player* nearest = nullptr;
		float nearestDist = FLT_MAX;
		for (auto* p : players)
		{
			if (p == nullptr) continue;
			const VECTOR pPos = p->GetTransform().pos;
			const float d = VSize(VSub(pPos, subject->GetTransform().pos));
			if (d < nearestDist)
			{
				nearestDist = d;
				nearest = p;
			}
		}

		if (nearest == nullptr) continue;

		const VECTOR nearestPos = nearest->GetTransform().pos;

		if (subject->CanStartAttack() && subject->IsInAttackRange(nearestPos))
		{
			subject->StartAttack(nearestPos);
		}

		if (subject->ConsumeAttackHit() && subject->IsInAttackRange(nearestPos))
		{
			nearest->TakeDamage(1);
		}
	}
}

bool GameScene::IsPlayerAlive(const Player* targetPlayer) const
{
	return targetPlayer != nullptr && !targetPlayer->IsDead();
}

bool GameScene::IsPlayerAtGoal(const Player* targetPlayer) const
{
	if (targetPlayer == nullptr)
	{
		return false;
	}

	VECTOR diff = VSub(targetPlayer->GetTransform().pos, GOAL_POS);
	diff.y = 0.0f;

	return VSize(diff) <= GOAL_RADIUS;
}

bool GameScene::IsPlayerReachedGoal(void) const
{
	bool hasAlivePlayer = false;

	for (const auto* player : players_)
	{
		if (!IsPlayerAlive(player))
		{
			continue;
		}

		hasAlivePlayer = true;

		if (!IsPlayerAtGoal(player))
		{
			return false;
		}
	}

	return hasAlivePlayer;
}

bool GameScene::IsAllPlayersDead(void) const
{
	if (players_.empty())
	{
		return false;
	}

	for (const auto* player : players_)
	{
		if (IsPlayerAlive(player))
		{
			return false;
		}
	}

	return true;
}

void GameScene::DrawGoalMarker(void) const
{
	const VECTOR spherePos = VAdd(GOAL_POS, VGet(0.0f, 45.0f, 0.0f));
	const VECTOR poleTop = VAdd(GOAL_POS, VGet(0.0f, 180.0f, 0.0f));
	const int ringColor = GetColor(0, 255, 120);

	DrawSphere3D(spherePos, GOAL_RADIUS, 16, ringColor, ringColor, FALSE);
	DrawLine3D(GOAL_POS, poleTop, ringColor);
}

void GameScene::DrawDeadView(
	int screenHandle,
	int drawWidth,
	int drawHeight,
	const char* playerName) const
{
	SetDrawScreen(screenHandle);
	SetDrawArea(0, 0, drawWidth, drawHeight);
	ClearDrawScreen();

	DrawBox(0, 0, drawWidth, drawHeight, GetColor(0, 0, 0), TRUE);

	DrawString(20, 20, playerName, GetColor(255, 255, 255));

	const int deadX = drawWidth / 2 - 50;
	const int deadY = drawHeight / 2 - 12;
	DrawString(deadX, deadY, "DEAD", GetColor(255, 60, 60));
}

