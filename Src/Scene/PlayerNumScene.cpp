#define NOMINMAX
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <io.h>     // _access
#include <stdlib.h> // _fullpath
#include "PlayerNumScene.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Object/Common/AnimationController.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ファイルローカルの効果音ハンドル（ヘッダを触らずに追加）
static int pn_moveSE = -1;
static int pn_toggleSE = -1;

// 軽量パーティクル構造体（ファイルローカル）
namespace
{
	struct Particle
	{
		float x, y;
		float vx, vy;
		float life;    // 残り寿命
		float maxLife; // 最大寿命
		float size;
		unsigned int color;
	};
	// プレイヤーごとのパーティクルコンテナ（4スロット）
	std::vector<Particle> g_particles[4];

	// プレイヤー毎のホログラム色（GetColorで取得）
	unsigned int GetPlayerColor(int idx)
	{
		switch (idx)
		{
		case 0: return GetColor(220, 80, 80);   // 赤
		case 1: return GetColor(80, 220, 200);  // 青緑
		case 2: return GetColor(80, 220, 100);  // 緑
		case 3: return GetColor(240, 220, 80);  // 黄
		default: return GetColor(200, 200, 200);
		}
	}

	// 全体下げ量（UI全体）
	constexpr int VERTICAL_OFFSET = 80;

	// 追加: モデルの表示スケール
	constexpr float MODEL_SCALE = 1.8f;

	// 左スティックでのカーソル移動判定用フラグ
	static bool s_padLeftStickMoved = false;
	// 左スティック閾値（InputManager と近い値を使う）
	constexpr float PAD_STICK_THRESHOLD = 0.35f;
}

namespace AnimType
{
	constexpr int IDLE = 0;
}

PlayerNumScene::PlayerNumScene(void)
{
	for (int i = 0; i < SELECT_MAX; ++i)
	{
		modelId_[i] = -1;
		animCtrl_[i] = nullptr;
		playerOffsetX_[i] = 0;
		playerOffsetY_[i] = 0;
	}
}

PlayerNumScene::~PlayerNumScene(void)
{
}

void PlayerNumScene::SetPlayerOffset(int idx, int offsetX, int offsetY)
{
	if (idx < 0 || idx >= SELECT_MAX) return;
	playerOffsetX_[idx] = offsetX;
	playerOffsetY_[idx] = offsetY;
}

void PlayerNumScene::GetPlayerOffset(int idx, int& outOffsetX, int& outOffsetY) const
{
	outOffsetX = outOffsetY = 0;
	if (idx < 0 || idx >= SELECT_MAX) return;
	outOffsetX = playerOffsetX_[idx];
	outOffsetY = playerOffsetY_[idx];
}

void PlayerNumScene::Init(void)
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	selectImg_[0] = LoadGraph("Data/PlayerNum/1P.png");
	selectImg_[1] = LoadGraph("Data/PlayerNum/2P.png");
	selectImg_[2] = LoadGraph("Data/PlayerNum/3P.png");
	selectImg_[3] = LoadGraph("Data/PlayerNum/4P.png");

	bgImg_ = LoadGraph("Data/PlayerNum/bg.png");
	if (bgImg_ == -1) bgImg_ = LoadGraph("Data/PlayerNum/bg.jpg");

	// 既存の決定SE（存在する場合）
	decideSE_ = LoadSoundMem("Data/PlayerNum/Sound/decide.mp3");
	// 追加効果音
	pn_moveSE = LoadSoundMem("Data/PlayerNum/Sound/move.mp3");
	pn_toggleSE = LoadSoundMem("Data/PlayerNum/Sound/toggle.mp3");

	selectNum_ = 0;
	cursor_ = 0;

	for (int i = 0; i < 4; i++)
	{
		isUsePlayer_[i] = false;
		g_particles[i].clear();
		playerOffsetX_[i] = 0;
		playerOffsetY_[i] = 0;
	}

	isUsePlayer_[0] = true;

	for (int i = 0; i < SELECT_MAX; ++i)
	{
		// --- 1. ベースモデルの読み込み ---
		char relModelPath[256];
		snprintf(relModelPath, sizeof(relModelPath), "Data/PlayerNum/Player.mv1");

		char fullPath[1024] = { 0 };
		if (_fullpath(fullPath, relModelPath, sizeof(fullPath)) != NULL && _access(fullPath, 0) == 0)
		{
			modelId_[i] = MV1LoadModel(fullPath);
		}
		else
		{
			modelId_[i] = MV1LoadModel(relModelPath);
		}

		if (modelId_[i] != -1)
		{
			MV1SetScale(modelId_[i], VGet(MODEL_SCALE, MODEL_SCALE, MODEL_SCALE));
			MV1SetRotationXYZ(modelId_[i], VGet(0.0f, 0.0f, 0.0f));

			// AnimationControllerの初期化
			animCtrl_[i] = new AnimationController(modelId_[i]);

			// --- 2. アニメーションファイルのパス解決（ベースモデル側とロジックを完全統一） ---
			char animPath[256];
			snprintf(animPath, sizeof(animPath), "Data/PlayerNum/Animation/Idle.mv1");

			char fullAnimPath[1024] = { 0 };
			const char* finalAnimPath = animPath;

			// 絶対パスに変換して存在するか確認
			if (_fullpath(fullAnimPath, animPath, sizeof(fullAnimPath)) != NULL && _access(fullAnimPath, 0) == 0)
			{
				finalAnimPath = fullAnimPath;
			}

			// コントローラー経由で登録してループ再生を開始
			animCtrl_[i]->Add(AnimType::IDLE, finalAnimPath, 1.0f);
			animCtrl_[i]->Play(AnimType::IDLE, true, 0.0f, -1.0f, false, true);
		}
	}
}

void PlayerNumScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	// 任意のパッド（PAD1..PAD4）をチェックするユーティリティ
	auto IsAnyPadBtnTrgDown = [&](InputManager::JOYPAD_BTN btn) -> bool {
		for (int p = static_cast<int>(InputManager::JOYPAD_NO::PAD1); p <= static_cast<int>(InputManager::JOYPAD_NO::PAD4); ++p)
		{
			if (ins.IsPadBtnTrgDown(static_cast<InputManager::JOYPAD_NO>(p), btn)) return true;
		}
		return false;
		};

	const auto padNo = InputManager::JOYPAD_NO::PAD1;

	// パッド状態を取得（左スティック操作対応を追加）
	const auto padState = ins.GetJPadInputState(padNo);
	// 左スティックで左右移動（トグルの移動）
	{
		const VECTOR stickDir = ins.GetDirXZAKey(padState.AKeyLX, padState.AKeyLY);
		// stickDir.x < 0 : 左、 >0 : 右
		if (std::fabs(stickDir.x) >= PAD_STICK_THRESHOLD)
		{
			if (!s_padLeftStickMoved)
			{
				// 一回だけ移動（スティックを倒した瞬間）
				if (stickDir.x < 0.0f)
				{
					cursor_--;
					if (cursor_ < 0) cursor_ = 3;
				}
				else
				{
					cursor_++;
					if (cursor_ > 3) cursor_ = 0;
				}
				// 移動音
				if (pn_moveSE != -1) PlaySoundMem(pn_moveSE, DX_PLAYTYPE_BACK);
				s_padLeftStickMoved = true;
			}
		}
		else
		{
			// スティックがニュートラルに戻ったら次の入力を受け付ける
			s_padLeftStickMoved = false;
		}
	}

	// スタートボタン（SPACE / 任意パッド LEFT）で決定
	const bool isStart =
		ins.IsTrgDown(KEY_INPUT_SPACE) ||
		ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::LEFT); // Xボタンで開始（遷移）

	const bool isToggle =
		ins.IsTrgDown(KEY_INPUT_RETURN) ||
		ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::DOWN); // Aボタンでトグル（決定）

	// 既存の D-PAD による左右移動も残す（補助）
	const bool isLeft =
		ins.IsTrgDown(KEY_INPUT_LEFT) ||
		ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::D_PAD_LEFT);

	const bool isRight =
		ins.IsTrgDown(KEY_INPUT_RIGHT) ||
		ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::D_PAD_RIGHT);

	if (isStart)
	{
		std::vector<int> selected;
		for (int i = 0; i < SELECT_MAX; ++i)
		{
			if (isUsePlayer_[i])
			{
				selected.push_back(i + 1);
			}
		}

		int playerCount = static_cast<int>(selected.size());
		if (playerCount <= 0)
		{
			playerCount = 1;
			selected.clear();
			selected.push_back(1);
		}

		scene.SetPlayerNum(playerCount);
		scene.SetSelectedPlayerNums(selected);

		PlaySoundMem(decideSE_, DX_PLAYTYPE_BACK);
		scene.ChangeScene(SceneManager::SCENE_ID::BUYSELECT);
	}

	// Enterキーで参加状態をトグル（確定）
	if (ins.IsTrgDown(KEY_INPUT_RETURN) || IsAnyPadBtnTrgDown(InputManager::JOYPAD_BTN::RIGHT))
	{
		isUsePlayer_[cursor_] = !isUsePlayer_[cursor_];
		// トグル音
		if (pn_toggleSE != -1) PlaySoundMem(pn_toggleSE, DX_PLAYTYPE_BACK);
	}

	// 左右カーソル
	if (isLeft)
	{
		cursor_--;
		if (cursor_ < 0) cursor_ = 3;
		// 移動音
		if (pn_moveSE != -1) PlaySoundMem(pn_moveSE, DX_PLAYTYPE_BACK);
	}
	if (isRight)
	{
		cursor_++;
		if (cursor_ > 3) cursor_ = 0;
		// 移動音
		if (pn_moveSE != -1) PlaySoundMem(pn_moveSE, DX_PLAYTYPE_BACK);
	}

	for (int i = 0; i < SELECT_MAX; ++i)
	{
		const int centerXBase = 250 + i * 250;
		const float centerX = static_cast<float>(centerXBase + playerOffsetX_[i]);
		const float baseY = 520.0f + static_cast<float>(playerOffsetY_[i]);

		int spawnCount = (i == cursor_) ? 5 : 1;
		for (int s = 0; s < spawnCount; ++s)
		{
			int r = std::rand() % 100;
			int threshold = (i == cursor_) ? 70 : 15;
			if (r < threshold)
			{
				Particle p;
				p.x = centerX + static_cast<float>(std::rand() % 51 - 25);
				p.y = baseY - static_cast<float>(std::rand() % 15);
				p.vx = (std::rand() % 101 - 50) * 0.003f;
				p.vy = -(0.8f + static_cast<float>(std::rand() % 100) * 0.005f);
				p.maxLife = 35.0f + static_cast<float>(std::rand() % 35);
				p.life = p.maxLife;
				p.size = 1.5f + (std::rand() % 6) * 0.4f;
				p.color = GetPlayerColor(i);
				g_particles[i].push_back(p);
			}
		}

		auto& vec = g_particles[i];
		for (size_t pi = 0; pi < vec.size();)
		{
			Particle& p = vec[pi];
			float wobble = (std::rand() % 101 - 50) * 0.001f;
			p.vx += wobble;
			p.x += p.vx;
			p.y += p.vy;
			p.vx *= 0.98f;
			p.life -= 1.0f;
			if (p.life <= 0.0f)
			{
				vec[pi] = vec.back();
				vec.pop_back();
			}
			else
			{
				++pi;
			}
		}

		if (animCtrl_[i])
		{
			animCtrl_[i]->Update();
		}
	}
}

void PlayerNumScene::Draw(void)
{
	int screenW = 0, screenH = 0;
	GetDrawScreenSize(&screenW, &screenH);

	// 背景描画
	if (bgImg_ != -1)
	{
		DrawExtendGraph(0, 0, screenW, screenH, bgImg_, FALSE);
	}
	else
	{
		DrawBox(0, 0, screenW, screenH, GetColor(12, 18, 30), TRUE);
	}

	// デバッグ表示
	/*{
		char dbgBuf[256];
		int selCount = 0;
		for (int i = 0; i < SELECT_MAX; ++i) if (isUsePlayer_[i]) ++selCount;
		snprintf(dbgBuf, sizeof(dbgBuf), "DEBUG Cursor:%d Selected:%d modelIds: %d,%d,%d,%d", cursor_, selCount,
			modelId_[0], modelId_[1], modelId_[2], modelId_[3]);
		DrawFormatString(10, 10, GetColor(255, 240, 120), "%s", dbgBuf);
	}*/

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 70);
	DrawBox(0, 0, screenW, screenH, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	DrawString(100, 70, "PLAYER SELECTION", GetColor(255, 255, 255));

	// 3Dカメラのセットアップと視野角の設定
	VECTOR camPos = VGet(0.0f, 60.0f, -480.0f);
	VECTOR camTarget = VGet(0.0f, 40.0f, 0.0f);
	SetCameraPositionAndTarget_UpVecY(camPos, camTarget);
	SetupCamera_Perspective(60.0f * static_cast<float>(M_PI) / 180.0f);

	// 3D描画の基本設定
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	for (int i = 0; i < SELECT_MAX; i++)
	{
		const int centerXBase = 250 + i * 250;
		const int centerX = centerXBase + playerOffsetX_[i];
		unsigned int pColor = GetPlayerColor(i);

		// --- 3Dモデル配置 ---
		if (modelId_[i] != -1)
		{
			float targetScreenX = static_cast<float>(centerX);
			float targetScreenY = 480.0f + static_cast<float>(playerOffsetY_[i]);

			VECTOR posNear = ConvScreenPosToWorldPos(VGet(targetScreenX, targetScreenY, 0.0f));
			VECTOR posFar = ConvScreenPosToWorldPos(VGet(targetScreenX, targetScreenY, 1.0f));

			VECTOR dir = VSub(posFar, posNear);

			VECTOR worldPos = VGet(0.0f, 0.0f, 0.0f);
			if (fabs(dir.z) > 0.0001f)
			{
				float t = -posNear.z / dir.z;
				worldPos.x = posNear.x + t * dir.x;
				worldPos.y = posNear.y + t * dir.y;
				worldPos.z = 0.0f;
			}
			else
			{
				worldPos = VGet(-240.0f + i * 160.0f, -60.0f, 0.0f);
			}

			// 座標を適用
			MV1SetPosition(modelId_[i], worldPos);

			// --- サイバーホログラムエフェクト設定 ---
			float r = static_cast<float>((pColor >> 16) & 0xFF) / 255.0f;
			float g = static_cast<float>((pColor >> 8) & 0xFF) / 255.0f;
			float b = static_cast<float>(pColor & 0xFF) / 255.0f;

			float alpha = 0.5f;
			float pulse = 1.0f;

			if (i == cursor_)
			{
				float angle = sinf(static_cast<float>(GetNowCount()) * 0.002f) * 0.15f;
				MV1SetRotationXYZ(modelId_[i], VGet(0.0f, angle, 0.0f));

				pulse = 1.0f + sinf(static_cast<float>(GetNowCount()) * 0.008f) * 0.15f;
				alpha = 0.85f + sinf(static_cast<float>(GetNowCount()) * 0.005f) * 0.1f;
			}
			else
			{
				MV1SetRotationXYZ(modelId_[i], VGet(0.0f, 0.0f, 0.0f));

				if (isUsePlayer_[i])
				{
					alpha = 0.6f;
					pulse = 0.9f;
				}
				else
				{
					alpha = 0.5f;
					pulse = 0.7f;
				}
			}

			COLOR_F colorScale;
			colorScale.r = r * pulse;
			colorScale.g = g * pulse;
			colorScale.b = b * pulse;
			colorScale.a = alpha;

			MV1SetDifColorScale(modelId_[i], colorScale);
			MV1SetOpacityRate(modelId_[i], alpha);

			// 🛠️ 描画直前の余分な Update() は削除しました
			// 透過重なりを防ぐためにZバッファの挙動を調整
			SetUseZBuffer3D(TRUE);
			SetWriteZBuffer3D(FALSE);

			// モデル描画
			MV1DrawModel(modelId_[i]);

			// 【後始末】次の3D描画のために書き込み設定を戻しておく
			SetWriteZBuffer3D(TRUE);
		}

		// --- 2D UI層 ---
		SetUseZBuffer3D(FALSE);
		SetWriteZBuffer3D(FALSE);

		// サイバーライン
		DrawBox(centerX - 80, 485, centerX + 80, 489, pColor, FALSE);

		// パネル背景
		const int panelLeft = centerX - 100;
		const int panelRight = centerX + 100;
		const int panelTop = 535;
		const int panelBottom = 583;

		if (i == cursor_)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 190);
			DrawBox(panelLeft, panelTop, panelRight, panelBottom, GetColor(5, 12, 22), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			DrawBox(panelLeft, panelTop, panelRight, panelBottom, pColor, FALSE);
		}
		else
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 110);
			DrawBox(panelLeft, panelTop, panelRight, panelBottom, GetColor(10, 14, 20), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			if (isUsePlayer_[i])
			{
				DrawBox(panelLeft, panelTop, panelRight, panelBottom, GetColor(0, 220, 120), FALSE);
			}
			else
			{
				DrawBox(panelLeft, panelTop, panelRight, panelBottom, GetColor(55, 65, 75), FALSE);
			}
		}

		// パーティクル
		for (const auto& p : g_particles[i])
		{
			float lifeRatio = p.life / p.maxLife;
			int alphaP = static_cast<int>(180.0f * lifeRatio);
			if (alphaP < 8) alphaP = 8;
			SetDrawBlendMode(DX_BLENDMODE_ADD, alphaP);
			DrawCircle(static_cast<int>(p.x), static_cast<int>(p.y), static_cast<int>(p.size), p.color, TRUE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// 文字列
		const int textX = panelLeft + 16;
		const int textY = panelTop + 15;

		if (i == cursor_)
		{
			bool blink = ((GetNowCount() / 250) % 2) == 0;
			unsigned int arrowColor = blink ? pColor : GetColor(0, 0, 0);

			char buf[64];
			snprintf(buf, sizeof(buf), "P%d SELECTING", i + 1);

			DrawFormatString(textX, textY, arrowColor, ">");
			DrawFormatString(textX + 16, textY, GetColor(255, 255, 255), "%s", buf);
		}
		else if (isUsePlayer_[i])
		{
			DrawFormatString(textX, textY, GetColor(0, 255, 150), "[ LOCKED IN ]");

			int frameCol = GetColor(0, 255, 140);
			DrawBox(panelLeft - 4, panelTop - 4, panelRight + 4, panelBottom + 4, frameCol, FALSE);
		}
		else
		{
			char buf[64];
			snprintf(buf, sizeof(buf), "[ P%d READY ]", i + 1);
			DrawFormatString(textX, textY, GetColor(100, 125, 135), "%s", buf);
		}

		if (i == cursor_)
		{
			bool blink = ((GetNowCount() / 200) % 2) == 0;
			unsigned int arrowColor = blink ? pColor : GetColor(120, 120, 120);
			DrawFormatString(centerX - 6, panelBottom + 8, arrowColor, "^");
		}

		SetUseZBuffer3D(TRUE);
		SetWriteZBuffer3D(TRUE);
	}

	DrawFormatString(
		40,
		640,
		GetColor(255, 255, 255),
		"RB : 人数選択");

	DrawFormatString(
		40,
		670,
		GetColor(255, 255, 255),
		"Bボタン         : 人数追加");

	DrawFormatString(
		40,
		700,
		GetColor(255, 255, 255),
		"Xボタン         : 決定");

	SetWriteZBuffer3D(FALSE);
	SetUseZBuffer3D(FALSE);
}

void PlayerNumScene::Release(void)
{
	// サウンド解放（ファイルローカル）
	if (pn_moveSE != -1) { DeleteSoundMem(pn_moveSE); pn_moveSE = -1; }
	if (pn_toggleSE != -1) { DeleteSoundMem(pn_toggleSE); pn_toggleSE = -1; }

	for (int i = 0; i < SELECT_MAX; i++)
	{
		if (selectImg_[i] != -1) DeleteGraph(selectImg_[i]);
		if (animCtrl_[i])
		{
			delete animCtrl_[i];
			animCtrl_[i] = nullptr;
		}
		if (modelId_[i] != -1)
		{
			MV1DeleteModel(modelId_[i]);
			modelId_[i] = -1;
		}
	}
	if (useImg_ != -1) DeleteGraph(useImg_);
	if (notUseImg_ != -1) DeleteGraph(notUseImg_);
	if (bgImg_ != -1) DeleteGraph(bgImg_);

	DeleteSoundMem(decideSE_);

	for (int i = 0; i < 4; ++i) g_particles[i].clear();
}