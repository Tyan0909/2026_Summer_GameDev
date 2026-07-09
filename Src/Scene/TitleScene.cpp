#define NOMINMAX
#include <cmath>
#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/Grid.h"
#include "../Object/UIInput.h"
#include "../Manager/Camera.h"
#include "../Application.h"
#include "TitleScene.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/SoundManager.h"

TitleScene::TitleScene(void) : SceneBase()
{
	debugGrid_ = nullptr;
}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{
	SoundManager::GetInstance().SetBgmVolume(180);
	SoundManager::GetInstance().PlayBgm(
		ResourceManager::SRC::BGM_TITLE);

	// カメラセット
	Camera* camera = SceneManager::GetInstance().GetCamera();
	camera->ChangeMode(Camera::MODE::FREE);

	// デバッググリッド
	debugGrid_ = new Grid();
	debugGrid_->Init();

	// 基本アセット読み込み
	logoHandle_ = LoadGraph("data/Image/Title/Title logo.png");
	pointerHandle_ = LoadGraph("data/Image/Title/pointer.png");
	shutterSE_ = LoadSoundMem("data/Sound/shutter.mp3");
	pressHandle_ = LoadGraph("data/Image/Title/press_space.png");

	// ホラー用アセット（無ければ -1 が返るが安全に扱う）
	ambientSE_ = LoadSoundMem("data/Sound/ambient_loop.mp3");
	whisperSE_ = LoadSoundMem("data/Sound/whisper.mp3");
	vignetteHandle_ = LoadGraph("data/Image/Title/vignette.png");
	fogHandle_ = LoadGraph("data/Image/Title/fog.png");
	bloodHandle_ = LoadGraph("data/Image/Title/blood.png");
	noiseHandle_ = LoadGraph("data/Image/Title/noise.png");

	// 状態初期化
	shutterScale_ = 1.0f;
	isShutter_ = false;
	flashAlpha_ = 0;
	fadeAlpha_ = 0;
	pointerAnim_ = 0.0f;
	logoAlpha_ = 0;

	pointerPos_ = VGet(200, 200, 0);
	targetPos_ = VGet(640, 360, 0);

	// ホラー初期値
	whisperTimer_ = 0;
	// 発生感覚を早める（初期遅延を短縮）
	nextWhisperDelay_ = GetRand(300) + 120; // 120～419フレーム
	vignetteAlpha_ = 120;
	fogAlpha_ = 80;
	bloodAlpha_ = 0;
	logoJitterX_ = 0.0f;
	logoJitterY_ = 0.0f;
	forceRedText_ = false;

	// ノイズ初期化
	noiseAlpha_ = 30;
	noiseOffsetX_ = 0.0f;
	noiseOffsetY_ = 0.0f;
	noiseTime_ = 0.0f;
	noiseSpeedX_ = 0.8f;
	noiseSpeedY_ = 0.25f;

	// スキャンライン / グリッチ初期化
	scanlineAlpha_ = 40.0f;
	isGlitching_ = false;
	glitchTimer_ = 0;
	glitchDuration_ = 0;

	// 環境音をループ再生（ロード成功時）
	if (ambientSE_ != -1)
	{
		PlaySoundMem(ambientSE_, DX_PLAYTYPE_LOOP);
	}

	SoundManager::GetInstance().PlayBgm(
		ResourceManager::SRC::BGM_TITLE);
}


static float RandFloat(float a, float b)
{
	return a + (GetRand(10000) / 10000.0f) * (b - a);
}

void TitleScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	// Enterでゲーム開始
	if (ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		PlaySoundMem(shutterSE_, DX_PLAYTYPE_BACK);
		scene.SetSplitScreenEnabled(false);
		scene.ChangeScene(SceneManager::SCENE_ID::GAME);
		return;
	}

	// Spaceでプレイ人数選択（カメラ演出付き）
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		scene.SetSplitScreenEnabled(true);

		isShutter_ = true;
		shutterScale_ = 1.0f;
		flashAlpha_ = 255;

		PlaySoundMem(shutterSE_, DX_PLAYTYPE_BACK);
	}

	// シャッター演出
	if (isShutter_)
	{
		shutterScale_ -= 0.08f;
		fadeAlpha_ += 12;

		if (shutterScale_ <= 0.0f)
		{
			shutterScale_ = 0.0f;
			isShutter_ = false;
			scene.ChangeScene(SceneManager::SCENE_ID::PLAYERNUMBERSELECT);
		}
	}

	// フラッシュの減衰
	if (flashAlpha_ > 0)
	{
		flashAlpha_ -= 20;
		if (flashAlpha_ < 0) flashAlpha_ = 0;
	}

	// ロゴのアルファ点滅（不安定化）
	if (GetRand(100) < 6)
	{
		logoAlpha_ = 30 + GetRand(80);
	}
	else
	{
		logoAlpha_ += 35;
		if (logoAlpha_ > 255) logoAlpha_ = 255;
	}

	// ポインタ追従
	pointerPos_.x += (targetPos_.x - pointerPos_.x) * 0.1f;
	pointerPos_.y += (targetPos_.y - pointerPos_.y) * 0.1f;

	// ポインタアニメ
	pointerAnim_ += 0.5f;

	// デバッググリッド更新
	debugGrid_->Update();

	// ささやき生成タイマー（感覚を早める）
	whisperTimer_++;
	if (whisperTimer_ > nextWhisperDelay_)
	{
		whisperTimer_ = 0;
		// 次回も比較的早めに発生するよう短いレンジにする
		nextWhisperDelay_ = GetRand(300) + 120; // 120～419フレーム

		// ささやき再生
		if (whisperSE_ != -1)
		{
			PlaySoundMem(whisperSE_, DX_PLAYTYPE_BACK);
		}
		// 赤テキストトリガー（短時間）
		forceRedText_ = true;

		// グリッチをトリガー（強いノイズ帯を数フレーム走らせる）
		isGlitching_ = true;
		glitchDuration_ = GetRand(30) + 30; // 30～59フレーム
		glitchTimer_ = glitchDuration_;
		// 少しスキャンラインを強めに
		scanlineAlpha_ = 120.0f;
	}

	// forceRedText_ を短時間だけ維持
	if (forceRedText_)
	{
		static int redDuration = 60;
		redDuration--;
		if (redDuration <= 0)
		{
			forceRedText_ = false;
			redDuration = 60;
		}
	}

	// 血しぶき関連（削除） -- 代わりに古い薄い血エフェクトを軽く残す
	if (GetRand(1000) < 8)
	{
		bloodAlpha_ = 30 + GetRand(140);
	}
	if (bloodAlpha_ > 0)
	{
		bloodAlpha_ -= 1;
		if (bloodAlpha_ < 0) bloodAlpha_ = 0;
	}

	// ノイズのオフセット / 時間を動かす（シェーダー風アニメーション用）
	noiseTime_ += 1.0f;
	noiseOffsetX_ += noiseSpeedX_ + (GetRand(100) / 100.0f) * 0.6f;
	noiseOffsetY_ += noiseSpeedY_ + (GetRand(100) / 100.0f) * 0.3f;
	if (noiseOffsetX_ > 640.0f) noiseOffsetX_ -= 640.0f;
	if (noiseOffsetY_ > 360.0f) noiseOffsetY_ -= 360.0f;

	// ノイズ強さをぽつぽつ変える
	noiseAlpha_ = 20 + (int)((std::sin(noiseTime_ * 0.08f) * 0.5f + 0.5f) * 60);

	// グリッチのタイマー処理（終われば通常へ）
	if (isGlitching_)
	{
		glitchTimer_--;
		if (glitchTimer_ <= 0)
		{
			isGlitching_ = false;
			// ゆっくりスキャンライン戻す
			scanlineAlpha_ = 40.0f;
		}
	}
	else
	{
		// スキャンラインをゆらぎで少し変化させる
		scanlineAlpha_ += (std::sin(noiseTime_ * 0.02f) * 0.5f + 0.5f) * 1.2f - 0.6f;
		if (scanlineAlpha_ < 8.0f) scanlineAlpha_ = 8.0f;
		if (scanlineAlpha_ > 140.0f) scanlineAlpha_ = 140.0f;
	}
}

void TitleScene::Draw(void)
{
	// 3Dデバッググリッド
	debugGrid_->Draw();

	// 背景を暗くする（ベースの暗転）
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 220);
	DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// ロゴ描画（固定位置）
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, logoAlpha_);
	if (logoHandle_ != -1)
	{
		const int logoX = 630;
		const int logoY = 300;
		DrawRotaGraph(logoX, logoY, 0.7, 0.0, logoHandle_, TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// ポインタ
	if (pointerHandle_ != -1)
	{
		DrawRotaGraph((int)pointerPos_.x, (int)pointerPos_.y, 1.0, 0.0, pointerHandle_, TRUE);
	}

	// フラッシュ（カメラ）
	if (flashAlpha_ > 0)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, flashAlpha_);
		DrawBox(0, 0, 1280, 720, GetColor(255, 255, 255), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// フォグ（スクロール風）
	if (fogHandle_ != -1)
	{
		int fogX = (int)((std::sin(pointerAnim_ * 0.02f) * 50.0f));
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, fogAlpha_);
		DrawRotaGraph(640 + fogX, 360, 1.2, 0.0, fogHandle_, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// （薄い）従来の血しぶきテクスチャは任意で残す（小エフェクト）
	if (bloodAlpha_ > 0 && bloodHandle_ != -1)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, bloodAlpha_);
		DrawRotaGraph(640, 360, 1.0, 0.0, bloodHandle_, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// ビネット（画面端を暗くする）
	if (vignetteHandle_ != -1)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, vignetteAlpha_);
		DrawRotaGraph(640, 360, 1.0, 0.0, vignetteHandle_, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// ----------------------------
	// ノイズ / テレビ風エフェクト描画
	// ----------------------------
	// ベースノイズレイヤー
	if (noiseHandle_ != -1 && noiseAlpha_ > 0)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, noiseAlpha_);
		// 大きめに引き伸ばして画面全体にタイル表示
		DrawRotaGraph(640 + (int)(noiseOffsetX_ * 0.6f), 360 + (int)(noiseOffsetY_ * 0.6f), 1.5, 0.0, noiseHandle_, TRUE);
		// 少し別オフセットのレイヤーを加えて動きを強める（加算）
		SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(noiseAlpha_ * 0.5f));
		DrawRotaGraph(640 - (int)(noiseOffsetX_ * 0.4f), 360 - (int)(noiseOffsetY_ * 0.4f), 1.4, 0.0, noiseHandle_, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// スキャンライン（細い横線でテレビ感）
	{
		int step = 3; // ライン間隔（px）
		int alpha = (int)scanlineAlpha_;
		if (alpha > 0)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			for (int y = 0; y < 720; y += step)
			{
				DrawBox(0, y, 1280, y + 1, GetColor(0, 0, 0), TRUE);
			}
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	// グリッチ帯（強いノイズの横帯）を重ねる
	if (isGlitching_ && noiseHandle_ != -1)
	{
		int bands = 2 + GetRand(3); // 2～4帯
		for (int i = 0; i < bands; ++i)
		{
			int bandY = GetRand(680) + 20;
			float scaleX = RandFloat(1.2f, 3.0f);
			float scaleY = RandFloat(0.12f, 0.5f);
			int alpha = 120 + GetRand(120);
			SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
			DrawRotaGraph(640, bandY, scaleX, 0.0, noiseHandle_, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			// 少し明暗差の矩形を加えてバンド感を出す
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha / 3);
			DrawBox(0, bandY - (int)(scaleY * 100.0f), 1280, bandY + (int)(scaleY * 100.0f), GetColor(40, 0, 0), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	// ノイズの小スクロールノイズを常時わずかに描画（微細グリッチ）
	if (noiseHandle_ != -1)
	{
		SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(noiseAlpha_ * 0.15f));
		DrawRotaGraph(640 + (int)(noiseOffsetX_ * 1.0f), 360 + (int)(noiseOffsetY_ * 1.2f), 0.9, 0.0, noiseHandle_, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// UI 描画（テキストを時折赤く）
	int uiColor = forceRedText_ ? GetColor(220, 40, 40) : GetColor(255, 255, 255);
	DrawString(200, 200, "タイトル - 不穏な気配が漂う", uiColor);
	DrawString(200, 240, "SPACE : ゆっくり進む", uiColor);
	DrawString(200, 280, "ENTER : 先に進む", uiColor);

	// PRESS SPACE 点滅
	int textAlpha = (int)((std::sin(pointerAnim_ * 0.05f) * 0.5f + 0.5f) * 255);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, textAlpha);
	if (pressHandle_ != -1)
	{
		DrawRotaGraph(590, 520, 0.35, 0.0, pressHandle_, TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void TitleScene::Release(void)
{
	// メモリ解放
	delete debugGrid_;
	debugGrid_ = nullptr;

	SoundManager::GetInstance().StopBgm();

	// 画像
	if (logoHandle_ != -1) DeleteGraph(logoHandle_);
	if (pointerHandle_ != -1) DeleteGraph(pointerHandle_);
	if (pressHandle_ != -1) DeleteGraph(pressHandle_);
	if (vignetteHandle_ != -1) DeleteGraph(vignetteHandle_);
	if (fogHandle_ != -1) DeleteGraph(fogHandle_);
	if (bloodHandle_ != -1) DeleteGraph(bloodHandle_);
	if (noiseHandle_ != -1) DeleteGraph(noiseHandle_);

	// サウンド（停止してから削除）
	if (ambientSE_ != -1)
	{
		StopSoundMem(ambientSE_);
		DeleteSoundMem(ambientSE_);
	}
	if (whisperSE_ != -1) DeleteSoundMem(whisperSE_);
	if (shutterSE_ != -1) DeleteSoundMem(shutterSE_);
}