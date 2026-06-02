#define NOMINMAX
#include "LoadingScene.h"
#include "../Manager/SceneManager.h"
#include <DxLib.h>
#include <cmath>
#include <random>

// 環境によっては未定義の可能性があるためフォールバックを用意
#ifndef DX_FONTTYPE_ANTIALIAS
#define DX_FONTTYPE_ANTIALIAS 0
#endif

// ファイルスコープのフォントハンドル（Init/Release で管理）
static int s_fontLarge = -1;
static int s_fontMid = -1;
static int s_fontSmall = -1;

// ロード表示用スプライト（最大7体）
static int s_unitImg[7] = { -1, -1, -1, -1, -1, -1, -1 };

LoadingScene::LoadingScene()
	: SceneBase()
{
}

LoadingScene::~LoadingScene(void)
{
}

void LoadingScene::Init(void)
{
	progress_ = 0.0f;
	isRequested_ = false;
	dummyHandle_ = -1;

	// 乱数初期化（パーティクル用）
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	// フォント作成（1回だけ）
	s_fontLarge = CreateFontToHandle(NULL, 36, -1, DX_FONTTYPE_ANTIALIAS);
	s_fontMid = CreateFontToHandle(NULL, 24, -1, DX_FONTTYPE_ANTIALIAS);
	s_fontSmall = CreateFontToHandle(NULL, 16, -1, DX_FONTTYPE_ANTIALIAS);

	// ユニット画像読み込み（Unit1..Unit7.png を優先、なければ Unit.png でフォールバック）
	for (int i = 0; i < 7; ++i)
	{
		char path[256];
		snprintf(path, sizeof(path), "Data/Image/Loading/Unit%d.png", i + 1);
		s_unitImg[i] = LoadGraph(path);
		if (s_unitImg[i] == -1)
		{
			// フォールバック単体画像
			if (s_unitImg[0] == -1)
			{
				s_unitImg[0] = LoadGraph("Data/Image/Loading/Unit.png");
			}
			// 使える画像があればそれを使う
			s_unitImg[i] = (s_unitImg[0] != -1) ? s_unitImg[0] : -1;
		}
	}
}

void LoadingScene::Update(void)
{
	// SceneManager から deltaTime を取得（安定化のため）
	float dt = SceneManager::GetInstance().GetDeltaTime();

	// プログレスは一定時間で進む（LOAD_DURATION はヘッダで定義済み）
	progress_ += dt / LOAD_DURATION;
	if (progress_ > 1.0f) progress_ = 1.0f;

	// 完了後に次シーンへ遷移
	if (progress_ >= 1.0f && !isRequested_)
	{
		isRequested_ = true;
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
	}
}

void LoadingScene::Draw(void)
{
	// 画面クリア
	ClearDrawScreen();

	// 背景：ダークグラデ + 軽いノイズを足す
	DrawBox(0, 0, 1280, 720, GetColor(6, 10, 18), TRUE);
	// 微かなグラデーション（上部をやや暗く）
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 40);
	DrawBox(0, 0, 1280, 120, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// タイトルテキスト（サイバー風）
	const int titleX = 640;
	const int titleY = 220;
	const unsigned int titleColor = GetColor(180, 240, 255);
	if (s_fontLarge != -1)
	{
		DrawFormatStringToHandle(titleX - 160, titleY - 18, titleColor, s_fontLarge, "LOADING");
	}
	else
	{
		DrawFormatString(titleX - 160, titleY - 18, titleColor, "LOADING");
	}
	// 中央の説明
	DrawFormatString(520, titleY + 26, GetColor(120, 200, 255), "Preparing cyber systems...");

	// プログレスバー位置とサイズ
	const int barX = 160;
	const int barY = 360;
	const int barW = 960;
	const int barH = 36;

	// バーの外枠（薄い枠）
	DrawBox(barX - 2, barY - 2, barX + barW + 2, barY + barH + 2, GetColor(180, 220, 255), FALSE);

	// グラデーションで塗る（背景レイヤ）
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 60);
	DrawBox(barX, barY, barX + barW, barY + barH, GetColor(8, 12, 20), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// 塗り部分（進捗に応じた幅）
	int filledW = static_cast<int>(barW * progress_);

	// グラデーション塗り（複数段で擬似グラデーション）
	for (int i = 0; i < 6; ++i)
	{
		float t = i / 5.0f;
		int r = static_cast<int>((1.0f - t) * 40 + t * 180);
		int g = static_cast<int>((1.0f - t) * 220 + t * 80);
		int b = static_cast<int>((1.0f - t) * 255 + t * 255);
		int alpha = static_cast<int>(200 * (1.0f - t * 0.6f));
		SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
		int inset = i; // 内側に少しずつ描画してぼかし風
		DrawBox(barX + inset, barY + inset, barX + std::max(0, filledW - inset), barY + barH - inset, GetColor(r, g, b), TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// スキャンライン（移動するハイライト）
	{
		const int scanW = 120; // ハイライト幅
		const float speed = 0.8f + (std::sin(GetNowCount() * 0.01f) + 1.0f) * 0.2f; // 速度を微振動
		float posNorm = std::fmod(GetNowCount() * 0.008f * speed + progress_ * 0.5f, 1.0f);
		int scanX = barX + static_cast<int>(posNorm * (barW + scanW)) - scanW;
		SetDrawBlendMode(DX_BLENDMODE_ADD, 140);
		for (int j = 0; j < 4; ++j)
		{
			int a = 60 - j * 12;
			if (a < 8) a = 8;
			int offset = j * 2;
			DrawBox(scanX - offset, barY - 1 - offset, scanX + scanW + offset, barY + barH + 1 + offset, GetColor(80, 220 - j * 20, 255), TRUE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// ロード進行に応じて表示するユニット数（0～7）
	int showCount = static_cast<int>(std::floor(progress_ * 7.0f + 1e-6f));
	if (showCount < 0) showCount = 0;
	if (showCount > 7) showCount = 7;

	// ユニットをバー上に均等配置して描画
	for (int u = 0; u < showCount; ++u)
	{
		int imgHandle = s_unitImg[u];
		// 配置の x（均等に barW 上に並べる）
		float unitCenterNorm = (u + 0.5f) / 7.0f; // 0..1
		int ux = barX + static_cast<int>(unitCenterNorm * barW);
		// y はバーの上に配置。少しボビンぐさせる
		int imgW = 32, imgH = 32;
		if (imgHandle != -1)
		{
			GetGraphSize(imgHandle, &imgW, &imgH);
		}
		int baseY = barY - imgH - 8;
		// ボビング
		float bob = std::sin((GetNowCount() * 0.02f) + u) * 4.0f;
		int uy = static_cast<int>(baseY + bob);

		if (imgHandle != -1)
		{
			// 中央合わせで描画
			DrawRotaGraph(ux, uy + imgH / 2, 1.0f, 0.0f, imgHandle, TRUE);
		}
		else
		{
			// プレースホルダ（小さなネオンサークル）
			SetDrawBlendMode(DX_BLENDMODE_ADD, 180);
			DrawCircle(ux, uy + (barH / 2), 8, GetColor(80, 200, 255), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	// 微小なパーティクル（バー上で漂う小点）
	{
		int seed = GetNowCount() / 4;
		for (int k = 0; k < 22; ++k)
		{
			int sx = barX + (k * 37 + (seed % 37)) % barW;
			int sy = barY + (std::sin((k + seed) * 0.13f) * (barH / 2 - 2)) + (barH / 2);
			int col = GetColor(100 + (k * 11) % 155, 180 + (k * 7) % 75, 230);
			int size = 1 + (k % 3 == 0);
			int alpha = 80 + static_cast<int>(std::abs(std::sin((GetNowCount() + k) * 0.02f)) * 120);
			SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
			DrawCircle(sx, sy - 8, size, col, TRUE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// 進捗パーセンテージ表示（大きく、ネオン縁取り）
	{
		char buf[64];
		sprintf_s(buf, "%3d %%", static_cast<int>(progress_ * 100.0f));

		// 外縁（グロー）
		if (s_fontMid != -1)
		{
			SetDrawBlendMode(DX_BLENDMODE_ADD, 120);
			DrawFormatStringToHandle(620 - 36, barY + barH + 18, GetColor(40, 220, 255), s_fontMid, "%s", buf);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else
		{
			SetDrawBlendMode(DX_BLENDMODE_ADD, 120);
			DrawFormatString(620 - 36, barY + barH + 18, GetColor(40, 220, 255), "%s", buf);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		// 中央文字（はっきり）
		DrawFormatString(622 - 36, barY + barH + 20, GetColor(10, 10, 20), "%s", buf);
	}

	// ちょっとしたスキャンテキスト（下部）
	DrawFormatString(520, barY + barH + 56, GetColor(130, 200, 255), "Est. time: calculating...");

	// 最後にフレームを反映
}

void LoadingScene::Release(void)
{
	// フォント解放
	if (s_fontLarge != -1) { DeleteFontToHandle(s_fontLarge); s_fontLarge = -1; }
	if (s_fontMid != -1) { DeleteFontToHandle(s_fontMid); s_fontMid = -1; }
	if (s_fontSmall != -1) { DeleteFontToHandle(s_fontSmall); s_fontSmall = -1; }

	// ユニット画像解放（同じハンドルが複数ある可能性があるため、ユニークに解放）
	for (int i = 0; i < 7; ++i)
	{
		if (s_unitImg[i] != -1)
		{
			// もし他のインデックスと同じハンドルなら二重解放を避ける
			bool unique = true;
			for (int j = 0; j < i; ++j) if (s_unitImg[j] == s_unitImg[i]) { unique = false; break; }
			if (unique) DeleteGraph(s_unitImg[i]);
			s_unitImg[i] = -1;
		}
	}

	// No-op（必要なら他リソース解放をここへ）
}