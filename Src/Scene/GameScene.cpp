#include <cmath>
#include <DxLib.h>
#include "GameScene.h"
#include "../Object/Actor/Stage/Stage.h"
#include "../Object/Actor/Charactor/Player/Player.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"

GameScene::GameScene()
	:
	stage_(nullptr),
	player_(nullptr),
	player2_(nullptr),
	leftScreenHandle_(-1),
	rightScreenHandle_(-1),
	screenWidth_(0),
	screenHeight_(0),
	player1CameraAngles_(VGet(0.0f, 0.0f, 0.0f)),
	player2CameraAngles_(VGet(0.0f, DX_PI_F, 0.0f))
{
}

GameScene::~GameScene()
{

}

void GameScene::Init()
{
	// ステージ初期化
	stage_ = new Stage();
	stage_->Init();

	// プレイヤー
	player_ = new Player();
	player_->Init();
	const ColliderBase* stageCollider =
		stage_->GetOwnCollider(static_cast<int>(Stage::COLLIDER_TYPE::MODEL));
	player_->AddHitCollider(stageCollider);

	player2_ = new Player();
	player2_->Init();
	player2_->SetInputEnabled(false);
	player2_->SetPos(PLAYER2_INIT_POS);
	player2_->AddHitCollider(stageCollider);

	GetDrawScreenSize(&screenWidth_, &screenHeight_);
	leftScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_, TRUE);
	rightScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_, TRUE);

}

void GameScene::Update()
{
	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	// スペースキーで結果シーンへ遷移
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		scene.ChangeScene(SceneManager::SCENE_ID::RESULT);
	}

	auto* camera = scene.GetCamera();
	camera->SetAngles(player1CameraAngles_);

	stage_->Update();
	player_->Update();
	player2_->Update();

	if (ins.IsNew(KEY_INPUT_UP))
	{
		player1CameraAngles_.x -= FPS_CAMERA_ROT_SPEED;
	}
	if (ins.IsNew(KEY_INPUT_DOWN))
	{
		player1CameraAngles_.x += FPS_CAMERA_ROT_SPEED;
	}
	if (ins.IsNew(KEY_INPUT_LEFT))
	{
		player1CameraAngles_.y -= FPS_CAMERA_ROT_SPEED;
	}
	if (ins.IsNew(KEY_INPUT_RIGHT))
	{
		player1CameraAngles_.y += FPS_CAMERA_ROT_SPEED;
	}
	if (player1CameraAngles_.x < FPS_CAMERA_PITCH_MIN)
	{
		player1CameraAngles_.x = FPS_CAMERA_PITCH_MIN;
	}
	if (player1CameraAngles_.x > FPS_CAMERA_PITCH_MAX)
	{
		player1CameraAngles_.x = FPS_CAMERA_PITCH_MAX;
	}
}

void GameScene::Draw()
{
	DrawSplitView(leftScreenHandle_, player_, player1CameraAngles_, player_);
	DrawSplitView(rightScreenHandle_, player2_, player2CameraAngles_, player2_);

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

void GameScene::DrawSplitView(int screenHandle, const Player* targetPlayer, const VECTOR& cameraAngles, const Player* hidePlayer)
{
	if (screenHandle == -1 || targetPlayer == nullptr)
	{
		return;
	}

	auto* camera = SceneManager::GetInstance().GetCamera();
	VECTOR eyeOffset = FPS_CAMERA_LOCAL_POS;
	eyeOffset = VTransform(eyeOffset, MGetRotY(cameraAngles.y));

	SetDrawScreen(screenHandle);
	ClearDrawScreen();
	SetDrawArea(0, 0, screenWidth_ / 2, screenHeight_);

	camera->SetPos(VAdd(targetPlayer->GetTransform().pos, eyeOffset));
	camera->SetAngles(cameraAngles);
	camera->SetBeforeDraw();

	stage_->Draw();
	if (player_ != hidePlayer)
	{
		player_->Draw();
	}
	if (player2_ != hidePlayer)
	{
		player2_->Draw();
	}
	DrawString(20, 20, targetPlayer == player_ ? "PLAYER 1" : "PLAYER 2", GetColor(255, 255, 255));
}

