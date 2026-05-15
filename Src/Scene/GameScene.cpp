#include <DxLib.h>
#include "GameScene.h"
#include "../Object/Actor/Stage/Stage.h"
#include "../Object/Actor/Charactor/Player/Player.h"
#include "../Object/Actor/Charactor/Subject/Subject.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"

GameScene::GameScene()
	:
	stage_(nullptr),
	player_(nullptr),
	player2_(nullptr),
	subject_(nullptr),
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

	subject_ = new Subject();
	subject_->Init();
	subject_->AddHitCollider(stageCollider);

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
	subject_->Update();

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

		// 一人称視点に戻す場合
		// DrawSingleView(player_, player_);
		return;
	}

	DrawSplitView(leftScreenHandle_, player_, nullptr);
	DrawSplitView(rightScreenHandle_, player2_, nullptr);

	// 一人称視点に戻す場合
	// DrawSplitView(leftScreenHandle_, player_, player_);
	// DrawSplitView(rightScreenHandle_, player2_, player2_);

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

	if (subject_)
	{
		subject_->Release();
		delete subject_;
		subject_ = nullptr;
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
	subject_->Draw();

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
	subject_->Draw();

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
	const VECTOR toSubject = VSub(targetSubject->GetTransform().pos, cameraPos);
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

	return dot >= PHOTO_SCORE_VIEW_DOT_MIN;
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
	if (player_ == nullptr || subject_ == nullptr)
	{
		return;
	}

	photoCount_++;

	if (!IsSubjectInView(player_, subject_))
	{
		lastPhotoScore_ = 0;
		return;
	}

	SceneManager& scene = SceneManager::GetInstance();

	const VECTOR shotPos = player_->GetCameraWorldPos();
	const VECTOR subjectPos = subject_->GetTransform().pos;
	const int addScore = CalculatePhotoScore(shotPos, subjectPos);

	lastPhotoScore_ = addScore;
	scene.SetCarryMoney(scene.GetCarryMoney() + addScore);
}

