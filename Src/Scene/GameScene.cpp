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
	screenWidth_(0),
	screenHeight_(0),
	isSplitScreenEnabled_(true),
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

	lastPhotoScore_ = 0;
	photoCount_ = 0;

	auto* camera = scene.GetCamera();
	camera->SetAngles(player_->GetCameraAngles());
	camera->ChangeMode(Camera::MODE::FREE);
}

void GameScene::Update()
{
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::RESULT);
	}

	stage_->Update();
	player_->Update();
	player2_->Update();

	if (subjectManager_ != nullptr)
	{
		subjectManager_->Update();
	}

	if (ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		TryTakePhoto();
	}
}

void GameScene::Draw()
{
	if (!isSplitScreenEnabled_)
	{
		DrawSingleView(player_, nullptr);
		return;
	}

	DrawSplitView(leftScreenHandle_, player_, nullptr);
	DrawSplitView(rightScreenHandle_, player2_, nullptr);

	SetDrawScreen(DX_SCREEN_BACK);
	SetDrawArea(0, 0, screenWidth_, screenHeight_);
	DrawGraph(0, 0, leftScreenHandle_, FALSE);
	DrawGraph(screenWidth_ / 2, 0, rightScreenHandle_, FALSE);
	DrawBox(screenWidth_ / 2 - 1, 0, screenWidth_ / 2 + 1, screenHeight_, GetColor(255, 255, 255), TRUE);
}

void GameScene::Draw3D()
{
	// 3D描画が必要な場合はここに追加
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
}

void GameScene::DrawSplitView(int screenHandle, const Player* targetPlayer, const Player* hidePlayer)
{
	if (screenHandle == -1 || targetPlayer == nullptr)
	{
		return;
	}

	auto* camera = SceneManager::GetInstance().GetCamera();

	SetDrawScreen(screenHandle);
	ClearDrawScreen();
	SetDrawArea(0, 0, screenWidth_ / 2, screenHeight_);

	camera->SetPos(targetPlayer->GetCameraWorldPos());
	camera->SetAngles(targetPlayer->GetCameraAngles());
	camera->SetBeforeDraw();

	stage_->Draw();

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

	DrawString(20, 20, targetPlayer == player_ ? "PLAYER 1" : "PLAYER 2", GetColor(255, 255, 255));
	DrawFormatString(20, 50, GetColor(255, 255, 0), "SCORE : %d", SceneManager::GetInstance().GetCarryMoney());
	DrawFormatString(20, 80, GetColor(0, 255, 255), "LAST PHOTO : +%d", lastPhotoScore_);
	DrawFormatString(20, 110, GetColor(255, 255, 255), "PHOTO COUNT : %d", photoCount_);
}

void GameScene::DrawSingleView(const Player* targetPlayer, const Player* hidePlayer)
{
	if (targetPlayer == nullptr)
	{
		return;
	}

	auto* camera = SceneManager::GetInstance().GetCamera();

	SetDrawScreen(DX_SCREEN_BACK);
	SetDrawArea(0, 0, screenWidth_, screenHeight_);

	camera->SetPos(targetPlayer->GetCameraWorldPos());
	camera->SetAngles(targetPlayer->GetCameraAngles());
	camera->SetBeforeDraw();

	stage_->Draw();

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

	DrawString(20, 20, "PLAYER 1", GetColor(255, 255, 255));
	DrawFormatString(20, 50, GetColor(255, 255, 0), "SCORE : %d", SceneManager::GetInstance().GetCarryMoney());
	DrawFormatString(20, 80, GetColor(0, 255, 255), "LAST PHOTO : +%d", lastPhotoScore_);
	DrawFormatString(20, 110, GetColor(255, 255, 255), "PHOTO COUNT : %d", photoCount_);
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

void GameScene::TryTakePhoto()
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

		// プレイヤー頭頂部と被写体頭頂部を3Dラインで結ぶ
		DrawLine3D(playerHeadPos, subjectHeadPos, lineColor);

		// 中点に距離を表示する
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