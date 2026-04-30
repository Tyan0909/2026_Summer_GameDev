#include <cmath>
#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/Grid.h"
#include "../Object/UIInput.h"
#include "../Manager/Camera.h"
#include "../Application.h"
#include "TitleScene.h"


TitleScene::TitleScene(void) : SceneBase()
{
    //grid_ = nullptr;
}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{
    // カメラモード変更
    Camera* camera = SceneManager::GetInstance().GetCamera();
    camera->ChangeMode(Camera::MODE::FREE);

    // デバッグ用グリッドの生成
    debugGrid_ = new Grid();
    debugGrid_->Init();

    logoHandle_ = LoadGraph("data/logo.png");
    pointerHandle_ = LoadGraph("data/pointer.png");
    shutterSE_ = LoadSoundMem("data/shutter.wav");

    shutterScale_ = 1.0f;
    isShutter_ = false;
    flashAlpha_ = 0;
    pointerPos_ = VGet(200, 200, 0);
    targetPos_ = VGet(640, 360, 0); // 画面中央
}

void TitleScene::Update(void)
{

    // シーン遷移
    InputManager& ins = InputManager::GetInstance();
    SceneManager& scene = SceneManager::GetInstance();

    // スペース押したら演出開始
    if (ins.IsTrgDown(KEY_INPUT_SPACE))
    {
        isShutter_ = true;
        shutterScale_ = 1.0f;
        flashAlpha_ = 255;
        PlaySoundMem(shutterSE_, DX_PLAYTYPE_BACK);
    }

    // シャッター閉じる
    if (isShutter_)
    {
        shutterScale_ -= 0.08f;

        if (shutterScale_ <= 0.0f)
        {
            shutterScale_ = 0.0f;
            isShutter_ = false;

            // 閉じたタイミングで遷移
            scene.ChangeScene(SceneManager::SCENE_ID::PLAYERNUMBERSELECT);

        }

    }

    // フラッシュ減衰
    if (flashAlpha_ > 0)
    {
        flashAlpha_ -= 20;
        if (flashAlpha_ < 0) flashAlpha_ = 0;
    }

    // ポインタ追従（ロックオン）
    pointerPos_.x += (targetPos_.x - pointerPos_.x) * 0.1f;
    pointerPos_.y += (targetPos_.y - pointerPos_.y) * 0.1f;

	// デバッグ用グリッドの更新
    debugGrid_->Update();

}

void TitleScene::Draw(void)

{

    // 3D
    debugGrid_->Draw();

    // ロゴ（中央）
    DrawRotaGraph(640, 360, shutterScale_, 0.0, logoHandle_, TRUE);

    // ポインタ
    DrawRotaGraph((int)pointerPos_.x, (int)pointerPos_.y, 1.0, 0.0, pointerHandle_, TRUE);

    // フラッシュ（最前面）
    if (flashAlpha_ > 0)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, flashAlpha_);
        DrawBox(0, 0, 1280, 720, GetColor(255, 255, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // タイトル画面の描画
    DrawString(200, 200, "タイトルシーン", GetColor(255, 255, 255));

    // デバッグ用グリッドの描画
    debugGrid_->Draw();
}

void TitleScene::Release(void)
{
	// デバッグ用グリッドの解放
    delete debugGrid_;
    debugGrid_ = nullptr;

    // 画像開放
    DeleteGraph(logoHandle_);
    DeleteGraph(pointerHandle_);
    DeleteSoundMem(shutterSE_);

}

