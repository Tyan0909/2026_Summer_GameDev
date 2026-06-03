#define NOMINMAX
#include <DxLib.h>
#include <vector>
#include <algorithm> // std::find, std::remove_if
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

// ファイルローカル: 設置効果音ハンドル
static int gs_placeSE = -1;
static int gs_explodeSE = -1; // 追加: 爆発音用ハンドル

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
	activePlayerCount_(1),
	iconHelmetHandle_(-1),
	iconFragHandle_(-1),
	iconSpikeHandle_(-1),
	iconMineHandle_(-1)
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

	// 追加: 各プレイヤーが持っている最初の使用可能アイテムを初期選択する
	for (auto* pl : players_)
	{
		if (pl) pl->CycleSelectedUsableItem(0); // dir==0 -> 最初の所持アイテムを選択
	}

	// 追加: BuySelect で購入したアイテムをプレイヤーに配布
	{
		const auto& purchased = SceneManager::GetInstance().GetPurchasedItemTypes();
		if (!purchased.empty() && !players_.empty())
		{
			for (size_t i = 0; i < purchased.size(); ++i)
			{
				int itemId = purchased[i];
				Player* target = players_[i % players_.size()];
				if (target)
				{
					target->AddItem(itemId);
				}
			}
			// 二重配布を防ぐためクリア
			SceneManager::GetInstance().SetPurchasedItemTypes(std::vector<int>{});
		}
	}

	// 追加: トラップ配列初期化
	traps_.clear();

	// 設置効果音ロード
	gs_placeSE = LoadSoundMem("Data/Sound/place.mp3");
	// 追加: 爆発音ロード
	gs_explodeSE = LoadSoundMem("Data/Sound/explode.mp3");

	// アイコンロード（複数パスを試す）
	auto TryLoadIcon = [](const std::vector<const char*>& paths) -> int {
		for (const char* p : paths)
		{
			int h = LoadGraph(p);
			if (h != -1) return h;
		}
		return -1;
	};

	iconHelmetHandle_ = TryLoadIcon({
		"Data/Image/Items/helmet.png",
		"Data/Image/items/helmet.png",
		"Data/Texture/Items/helmet.png",
		"Data/Texture/items/helmet.png",
		"Data/Items/helmet.png"
	});
	iconFragHandle_ = TryLoadIcon({
		"Data/Image/Items/frag.png",
		"Data/Image/items/frag.png",
		"Data/Texture/Items/frag.png",
		"Data/Texture/items/frag.png",
		"Data/Items/frag.png"
	});
	iconSpikeHandle_ = TryLoadIcon({
		"Data/Image/Items/spike.png",
		"Data/Image/items/spike.png",
		"Data/Texture/Items/spike.png",
		"Data/Texture/items/spike.png",
		"Data/Items/spike.png"
	});
	iconMineHandle_ = TryLoadIcon({
		"Data/Image/Items/mine.png",
		"Data/Image/items/mine.png",
		"Data/Texture/Items/mine.png",
		"Data/Texture/items/mine.png",
		"Data/Items/mine.png"
	});

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

// Update: Tab で選択アイテム切替、E で選択アイテムを使用
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

	// 選択アイテム切替 (Tab)
	if (ins.IsTrgDown(KEY_INPUT_TAB) && player_ != nullptr)
	{
		player_->CycleSelectedUsableItem(1);
	}

	// 追加: トラップ等使用（Eキー）: 選択中アイテムを使う
	if (ins.IsTrgDown(KEY_INPUT_E) && player_ != nullptr)
	{
		// 選択未設定なら所持アイテムから自動選択
		ITEM_TYPE currentSel = player_->GetSelectedUsableItemType();
		if (currentSel == ITEM_TYPE::NORMAL_CAMERA)
		{
			player_->CycleSelectedUsableItem(0); // 所持があれば最初のものを選ぶ
		}

		const ITEM_TYPE sel = player_->GetSelectedUsableItemType();
		if (sel == ITEM_TYPE::NORMAL_CAMERA)
		{
			// 所持している使用可能アイテムが無ければ何もしない
		}
		else
		{
			const VECTOR ppos = player_->GetTransform().pos;
			VECTOR forward = player_->GetCameraForward();
			// 発射位置 / 設置位置はプレイヤーの前方へ
			VECTOR explPos = VAdd(ppos, VScale(forward, 140.0f));

			switch (sel)
			{
			case ITEM_TYPE::SPIKE_TRAP:
				if (player_->UseSpikeTrap())
				{
					Trap t;
					t.type = TRAP_TYPE::SPIKE;
					t.pos = explPos;
					t.triggered = false;
					t.lifeFrames = SPIKE_DURATION_FRAMES;
					traps_.push_back(t);
					if (gs_placeSE != -1) PlaySoundMem(gs_placeSE, DX_PLAYTYPE_BACK);

					// 使用後、在庫が尽きていれば次の所持アイテムへ自動で切換
					if (player_->GetSpikeCount() <= 0)
					{
						player_->CycleSelectedUsableItem(1);
					}
				}
				break;
			case ITEM_TYPE::EXPLOSIVE_TRAP:
				if (player_->UseExplosiveTrap())
				{
					Trap t;
					t.type = TRAP_TYPE::MINE;
					t.pos = explPos; // プレイヤー前方に設置する
					t.triggered = false;
					t.lifeFrames = 0;
					traps_.push_back(t);
					if (gs_placeSE != -1) PlaySoundMem(gs_placeSE, DX_PLAYTYPE_BACK);

					if (player_->GetMineCount() <= 0)
					{
						player_->CycleSelectedUsableItem(1);
					}
				}
				break;
			case ITEM_TYPE::FRAG_GRENADE:
				if (player_->UseFragGrenade())
				{
					const float radius = 120.0f;
					if (subjectManager_ != nullptr)
					{
						auto& subs = const_cast<std::vector<Subject*>&>(subjectManager_->GetSubjects());
						for (auto it = subs.begin(); it != subs.end(); )
						{
							Subject* s = *it;
							if (s == nullptr) { ++it; continue; }
							VECTOR diff = VSub(s->GetTransform().pos, explPos);
							diff.y = 0.0f;
							if (VSize(diff) <= radius)
							{
								subjectManager_->RemoveSubject(s);
								it = subs.begin();
								continue;
							}
							++it;
						}
					}
					for (auto* pl : players_)
					{
						if (!pl) continue;
						VECTOR pd = VSub(pl->GetTransform().pos, explPos);
						pd.y = 0.0f;
						if (VSize(pd) <= radius)
						{
							int dmg = pl->GetHpMax() / 3;
							if (dmg < 1) dmg = 1;
							pl->TakeDamage(dmg);
						}
					}
					if (gs_placeSE != -1) PlaySoundMem(gs_placeSE, DX_PLAYTYPE_BACK);

					if (player_->GetFragCount() <= 0)
					{
						player_->CycleSelectedUsableItem(1);
					}
				}
				break;
			default:
				break;
			}
		}
	}

	// トラップ効果の判定（subjects_ を参照）
	if (!traps_.empty() && subjectManager_ != nullptr)
	{
		auto& subjects = const_cast<std::vector<Subject*>&>(subjectManager_->GetSubjects());
		for (auto& trap : traps_)
		{
			if (trap.type == TRAP_TYPE::SPIKE)
			{
				if (!trap.triggered)
				{
					for (auto* s : subjects)
					{
						if (s == nullptr) continue;
						const VECTOR spos = s->GetTransform().pos;
						VECTOR diff = VSub(spos, trap.pos);
						diff.y = 0.0f;
						if (VSize(diff) <= SPIKE_TRIGGER_RADIUS)
						{
							// スタン（4秒)
							s->Stun(SPIKE_DURATION_FRAMES);
							trap.triggered = true;
							trap.lifeFrames = SPIKE_DURATION_FRAMES;
							break;
						}
					}
				}
				else
				{
					trap.lifeFrames--;
				}
			}
			else // MINE
			{
				if (!trap.triggered)
				{
					for (auto* s : subjects)
					{
						if (s == nullptr) continue;
						const VECTOR spos = s->GetTransform().pos;
						VECTOR diff = VSub(spos, trap.pos);
						diff.y = 0.0f;
						if (VSize(diff) <= MINE_TRIGGER_RADIUS)
						{
							// 爆発: その Subject を削除（簡易）、近くのプレイヤーへダメージ
							subjectManager_->RemoveSubject(s);

							for (auto* pl : players_)
							{
								if (pl == nullptr) continue;
								const VECTOR ppos = pl->GetTransform().pos;
								VECTOR pdiff = VSub(ppos, trap.pos);
								pdiff.y = 0.0f;
								if (VSize(pdiff) <= MINE_DAMAGE_RADIUS)
								{
									int dmg = pl->GetHpMax() / 3;
									if (dmg <= 0) dmg = 1;
									pl->TakeDamage(dmg);
								}
							}

							trap.triggered = true;
							trap.lifeFrames = 30; // 爆発エフェクトを少し残す

							// 爆発音を再生
							if (gs_explodeSE != -1)
							{
								PlaySoundMem(gs_explodeSE, DX_PLAYTYPE_BACK);
							}

							break;
						}
					}
				}
				else
				{
					trap.lifeFrames--;
				}
			}
		}

		// 期限切れのトラップを消す
		traps_.erase(
			std::remove_if(traps_.begin(), traps_.end(), [](const Trap& t) { return t.triggered && t.lifeFrames <= 0; }),
			traps_.end());
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

	if (isScreenshotRequested_) CaptureScreenshot();

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
		if (hasScreenshot_) { DrawScreenshotThumbnail(); DrawString(20, 140, "F1 : OPEN SCREENSHOT", GetColor(255, 255, 255)); }
	}

	DrawFlashEffect();
}

void GameScene::DrawInventoryHUD(const Player* targetPlayer, int drawWidth, int drawHeight) const
{
	if (!targetPlayer) return;

	const int iconSize = ITEM_ICON_SIZE;
	const int spacing = ITEM_ICON_SPACING;
	const int margin = ITEM_ICON_MARGIN;

	// 4アイコン表示: HELMET, FRAG, SPIKE, MINE
	const int totalIcons = 4;
	const int totalWidth = iconSize * totalIcons + spacing * (totalIcons - 1);
	int x = drawWidth - margin - totalWidth;
	int y = margin;

	auto DrawIconOrFallback = [&](int handle, const char* label, int count)
	{
		if (handle != -1)
		{
			DrawExtendGraph(x, y, x + iconSize, y + iconSize, handle, TRUE);
		}
		else
		{
			// フォールバック: 色付きボックス + ラベル
			DrawBox(x, y, x + iconSize, y + iconSize, GetColor(40, 40, 40), TRUE);
			DrawBox(x, y, x + iconSize, y + iconSize, GetColor(200, 200, 200), FALSE);
			DrawFormatString(x + 6, y + iconSize / 2 - 6, GetColor(220, 220, 220), "%s", label);
		}
		DrawFormatString(x, y + iconSize + 4, GetColor(255, 255, 255), "x%d", count);
	};

	// HELMET (表示は残数、選択ハイライト対象外)
	DrawIconOrFallback(iconHelmetHandle_, "HLM", targetPlayer->GetHelmetUses());
	x += iconSize + spacing;

	// FRAG
	DrawIconOrFallback(iconFragHandle_, "FRG", targetPlayer->GetFragCount());
	if (targetPlayer->GetSelectedUsableItemType() == ITEM_TYPE::FRAG_GRENADE)
	{
		DrawBox(x - (iconSize + spacing) - 3, y - 3, x - (iconSize + spacing) - 3 + iconSize + 6, y + iconSize + 3, GetColor(255, 220, 80), FALSE);
	}
	// 現在の x は frag の右側にあるため選択ハイライト描画は上と少し工夫している
	x += iconSize + spacing;

	// SPIKE
	DrawIconOrFallback(iconSpikeHandle_, "SPK", targetPlayer->GetSpikeCount());
	if (targetPlayer->GetSelectedUsableItemType() == ITEM_TYPE::SPIKE_TRAP)
	{
		DrawBox(x - 3, y - 3, x + iconSize + 3, y + iconSize + 3, GetColor(255, 220, 80), FALSE);
	}
	x += iconSize + spacing;

	// MINE
	DrawIconOrFallback(iconMineHandle_, "MNE", targetPlayer->GetMineCount());
	if (targetPlayer->GetSelectedUsableItemType() == ITEM_TYPE::EXPLOSIVE_TRAP)
	{
		DrawBox(x - 3, y - 3, x + iconSize + 3, y + iconSize + 3, GetColor(255, 220, 80), FALSE);
	}
}

// DrawView の HUD 描画部分で呼び出すように、DrawView の末尾近くに既にある HUD 部分で DrawInventoryHUD(targetPlayer, drawWidth, drawHeight) を呼んでいます。

void GameScene::Release()
{
	if (player_) { player_->Release(); delete player_; player_ = nullptr; }
	if (player2_) { player2_->Release(); delete player2_; player2_ = nullptr; }
	if (player3_) { player3_->Release(); delete player3_; player3_ = nullptr; }
	if (player4_) { player4_->Release(); delete player4_; player4_ = nullptr; }

	if (subjectManager_) { subjectManager_->Release(); delete subjectManager_; subjectManager_ = nullptr; }
	if (stage_) { stage_->Release(); delete stage_; stage_ = nullptr; }

	if (leftScreenHandle_ != -1) { DeleteGraph(leftScreenHandle_); leftScreenHandle_ = -1; }
	if (rightScreenHandle_ != -1) { DeleteGraph(rightScreenHandle_); rightScreenHandle_ = -1; }
	if (bottomLeftScreenHandle_ != -1) { DeleteGraph(bottomLeftScreenHandle_); bottomLeftScreenHandle_ = -1; }
	if (bottomRightScreenHandle_ != -1) { DeleteGraph(bottomRightScreenHandle_); bottomRightScreenHandle_ = -1; }
	if (sceneScreenHandle_ != -1) { DeleteGraph(sceneScreenHandle_); sceneScreenHandle_ = -1; }
	if (screenshotScreenHandle_ != -1) { DeleteGraph(screenshotScreenHandle_); screenshotScreenHandle_ = -1; }

	if (iconHelmetHandle_ != -1) { DeleteGraph(iconHelmetHandle_); iconHelmetHandle_ = -1; }
	if (iconFragHandle_ != -1) { DeleteGraph(iconFragHandle_); iconFragHandle_ = -1; }
	if (iconSpikeHandle_ != -1) { DeleteGraph(iconSpikeHandle_); iconSpikeHandle_ = -1; }
	if (iconMineHandle_ != -1) { DeleteGraph(iconMineHandle_); iconMineHandle_ = -1; }

	if (gs_placeSE != -1) { DeleteSoundMem(gs_placeSE); gs_placeSE = -1; }
	if (gs_explodeSE != -1) { DeleteSoundMem(gs_explodeSE); gs_explodeSE = -1; } // 追加: 爆発音ハンドルを削除

	traps_.clear();
}

void GameScene::DrawView(
	int screenHandle,
	int drawWidth,
	int drawHeight,
	const Player* targetPlayer,
	const Player* hidePlayer,
	const char* playerName)
{
	if (screenHandle == -1 || targetPlayer == nullptr) return;
	if (!IsPlayerAlive(targetPlayer)) { DrawDeadView(screenHandle, drawWidth, drawHeight, playerName); return; }

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

	if (subjectManager_) subjectManager_->Draw();

	DrawSubjectDistanceGuide(targetPlayer);

	if (player_ != hidePlayer && player_) player_->Draw();
	if (player2_ != hidePlayer && player2_) player2_->Draw();
	if (player3_ != hidePlayer && player3_) player3_->Draw();
	if (player4_ != hidePlayer && player4_) player4_->Draw();

	// トラップ描画（球で近似）
	for (const auto& trap : traps_)
	{
		const VECTOR top = VAdd(trap.pos, VGet(0.0f, 40.0f, 0.0f));
		const VECTOR bottom = VAdd(trap.pos, VGet(0.0f, 2.0f, 0.0f));
		int color = (trap.type == TRAP_TYPE::SPIKE) ? GetColor(0, 200, 80) : GetColor(220, 80, 60);
		int fillColor = color;
		if (trap.triggered) { color = GetColor(255, 200, 80); fillColor = GetColor(255, 120, 50); }
		const int segCount = 8; const float radius = 12.0f;
		for (int si = 0; si < segCount; ++si)
		{
			const float t = (segCount == 1) ? 0.0f : static_cast<float>(si) / (segCount - 1);
			VECTOR p = VAdd(bottom, VScale(VSub(top, bottom), t));
			SetDrawBlendMode(DX_BLENDMODE_ADD, 160);
			DrawSphere3D(p, radius, 8, fillColor, fillColor, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		// トラップ範囲をワイヤーで強調
		{
			const VECTOR top = VAdd(trap.pos, VGet(0.0f, 40.0f, 0.0f));
			const VECTOR bottom = VAdd(trap.pos, VGet(0.0f, 2.0f, 0.0f));
			const int outline = GetColor(255, 255, 255);
			const int ringOutline = GetColor(255, 220, 120);

			// 軸線を描く
			DrawLine3D(bottom, top, ringOutline);

			// 主要部のワイヤー球を描く
			DrawSphere3D(top, 12.5f, 10, outline, outline, FALSE);
			DrawSphere3D(bottom, 12.5f, 10, outline, outline, FALSE);
		}
	}

	// --- 追加: プレビューカプセル（各ビューに表示） ---
	if (targetPlayer->GetSelectedUsableItemType() != ITEM_TYPE::NORMAL_CAMERA)
	{
		ITEM_TYPE previewType = targetPlayer->GetSelectedUsableItemType();
		int previewCount = 0;
		float previewRadius = 0.0f;
		int previewColor = GetColor(100, 200, 255);

		switch (previewType)
		{
		case ITEM_TYPE::SPIKE_TRAP:
			previewCount = targetPlayer->GetSpikeCount();
			previewRadius = SPIKE_TRIGGER_RADIUS;
			previewColor = GetColor(0, 200, 80);
			break;
		case ITEM_TYPE::EXPLOSIVE_TRAP:
			previewCount = targetPlayer->GetMineCount();
			previewRadius = MINE_TRIGGER_RADIUS;
			previewColor = GetColor(220, 80, 60);
			break;
		case ITEM_TYPE::FRAG_GRENADE:
			previewCount = targetPlayer->GetFragCount();
			previewRadius = 60.0f; // 小さくして見やすく
			previewColor = GetColor(255, 180, 60);
			break;
		default:
			break;
		}

		if (previewCount > 0 && previewRadius > 0.0f)
		{
			VECTOR ppos = targetPlayer->GetTransform().pos;
			// プレイス位置がプレイヤーの前方で見やすいように少し前へオフセット
			VECTOR forward = targetPlayer->GetCameraForward();
			ppos = VAdd(ppos, VScale(forward, 24.0f));
			ppos.y += 7.0f;

			// 半透明でプレビューを描画（球列でカプセル風）
			const int segCount = 10;
			const float height = 20.0f;
			const float radius = previewRadius;

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
			for (int si = 0; si < segCount; ++si)
			{
				float t = (segCount == 1) ? 0.0f : static_cast<float>(si) / (segCount - 1);
				VECTOR center = VAdd(ppos, VGet(0.0f, t * height, 0.0f));
				DrawSphere3D(center, radius, 12, previewColor, previewColor, TRUE);
			}
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

			// 輪郭をワイヤーで描く（見やすさ向上）
			const int outlineColor = GetColor(255, 255, 255);
			for (int si = 0; si < 2; ++si)
			{
				const float r = radius + 2.0f + si * 1.5f;
				VECTOR top = VAdd(ppos, VGet(0.0f, height, 0.0f));
				VECTOR bottom = ppos;
				DrawSphere3D(top, r, 8, outlineColor, outlineColor, FALSE);
				DrawSphere3D(bottom, r, 8, outlineColor, outlineColor, FALSE);
			}
		}
	}
	// --- プレビュー終わり ---

	// HUD
	DrawString(20, 20, playerName, GetColor(255, 255, 255));
	DrawFormatString(20, 50, GetColor(255, 255, 0), "SCORE : %d", SceneManager::GetInstance().GetCarryMoney());

	{
		const int barX = 20; const int barY = 145;
		const int barWidth = drawWidth >= 900 ? 240 : 180; const int barHeight = 18;
		const int backColor = GetColor(40, 40, 40);
		const int frameColor = GetColor(255, 255, 255);
		const int hpColor = GetColor(80, 220, 80);
		const int damageColor = GetColor(255, 90, 90);
		const int fillWidth = static_cast<int>(barWidth * targetPlayer->GetHpRate());
		const int currentColor = targetPlayer->CanTakeDamage() ? hpColor : damageColor;

		DrawString(barX, barY, "HP", GetColor(255, 255, 255));
		DrawBox(barX, barY + 22, barX + barWidth, barY + 22 + barHeight, backColor, TRUE);
		if (fillWidth > 0) DrawBox(barX, barY + 22, barX + fillWidth, barY + 22 + barHeight, currentColor, TRUE);
		DrawBox(barX, barY + 22, barX + barWidth, barY + 22 + barHeight, frameColor, FALSE);
		DrawFormatString(barX + barWidth + 12, barY + 22, GetColor(255, 255, 255), "%d / %d", targetPlayer->GetHp(), targetPlayer->GetHpMax());
	}

	int localLast = lastPhotoScore_; int localCount = 0;
	auto it = std::find(players_.begin(), players_.end(), targetPlayer);
	if (it != players_.end()) { const int idx = static_cast<int>(std::distance(players_.begin(), it)); localLast = lastPhotoScorePerPlayer_[idx]; localCount = photoCountPerPlayer_[idx]; }
	else { localCount = photoCount_; }

	DrawFormatString(20, 80, GetColor(0, 255, 255), "LAST PHOTO : +%d", localLast);
	DrawFormatString(20, 110, GetColor(255, 255, 255), "PHOTO COUNT : %d", localCount);

	// 追加: 選択中アイテムと残数表示
	ITEM_TYPE sel = targetPlayer->GetSelectedUsableItemType();
	const char* selName = "";
	int selCount = 0;
	switch (sel)
	{
	case ITEM_TYPE::FRAG_GRENADE: selName = "FRAG"; selCount = targetPlayer->GetFragCount(); break;
	case ITEM_TYPE::SPIKE_TRAP: selName = "SPIKE"; selCount = targetPlayer->GetSpikeCount(); break;
	case ITEM_TYPE::EXPLOSIVE_TRAP: selName = "MINE"; selCount = targetPlayer->GetMineCount(); break;
	default: selName = "NONE"; selCount = 0; break;
	}
	DrawFormatString(drawWidth - 220, 20, GetColor(180, 240, 255), "ITEM: %s x%d", selName, selCount);

	DrawInventoryHUD(targetPlayer, drawWidth, drawHeight);
}

void GameScene::DrawDeadView(int screenHandle, int drawWidth, int drawHeight, const char* playerName) const
{
	if (screenHandle == -1) return;

	SetDrawScreen(screenHandle);
	SetDrawArea(0, 0, drawWidth, drawHeight);
	ClearDrawScreen();

	// 背景を暗くして被写界深度っぽく
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
	DrawBox(0, 0, drawWidth, drawHeight, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// プレイヤー名表示
	DrawString(20, 20, playerName, GetColor(255, 255, 255));

	// 大きく DEATH 表示
	const int msgX = drawWidth / 2 - 72;
	const int msgY = drawHeight / 2 - 24;
	DrawFormatString(msgX, msgY, GetColor(255, 60, 60), "DEAD");

	// 小さな操作案内
	DrawFormatString(msgX - 40, msgY + 36, GetColor(200, 200, 200), "Press ENTER to continue");
}

void GameScene::DrawCompositedScene(void)
{
	if (sceneScreenHandle_ == -1) return;

	if (!isSplitScreenEnabled_ || activePlayerCount_ <= 1)
	{
		DrawView(sceneScreenHandle_, screenWidth_, screenHeight_, player_, nullptr, "PLAYER 1");
		return;
	}

	if (activePlayerCount_ == 2)
	{
		DrawView(leftScreenHandle_, screenWidth_ / 2, screenHeight_, player_, nullptr, "PLAYER 1");
		DrawView(rightScreenHandle_, screenWidth_ / 2, screenHeight_, player2_, nullptr, "PLAYER 2");

		SetDrawScreen(sceneScreenHandle_); SetDrawArea(0, 0, screenWidth_, screenHeight_); ClearDrawScreen();
		DrawGraph(0, 0, leftScreenHandle_, FALSE);
		DrawGraph(screenWidth_ / 2, 0, rightScreenHandle_, FALSE);
		DrawBox(screenWidth_ / 2 - 1, 0, screenWidth_ / 2 + 1, screenHeight_, GetColor(255, 255, 255), TRUE);
		return;
	}

	DrawView(leftScreenHandle_, screenWidth_ / 2, screenHeight_ / 2, player_, nullptr, "PLAYER 1");
	DrawView(rightScreenHandle_, screenWidth_ / 2, screenHeight_ / 2, player2_ ? player2_ : player_, nullptr, "PLAYER 2");
	DrawView(bottomLeftScreenHandle_, screenWidth_ / 2, screenHeight_ / 2, player3_ ? player3_ : player_, nullptr, "PLAYER 3");

	if (player4_)
	{
		DrawView(bottomRightScreenHandle_, screenWidth_ / 2, screenHeight_ / 2, player4_, nullptr, "PLAYER 4");
	}
	else
	{
		SetDrawScreen(bottomRightScreenHandle_); SetDrawArea(0, 0, screenWidth_ / 2, screenHeight_ / 2); ClearDrawScreen();
		DrawBox(0, 0, screenWidth_ / 2, screenHeight_ / 2, GetColor(10, 10, 10), TRUE);
		DrawFormatString(screenWidth_ / 4 - 40, screenHeight_ / 4 - 8, GetColor(180, 180, 180), "NO PLAYER");
	}

	SetDrawScreen(sceneScreenHandle_); SetDrawArea(0, 0, screenWidth_, screenHeight_); ClearDrawScreen();
	DrawGraph(0, 0, leftScreenHandle_, FALSE);
	DrawGraph(screenWidth_ / 2, 0, rightScreenHandle_, FALSE);
	DrawGraph(0, screenHeight_ / 2, bottomLeftScreenHandle_, FALSE);
	DrawGraph(screenWidth_ / 2, screenHeight_ / 2, bottomRightScreenHandle_, FALSE);

	DrawBox(screenWidth_ / 2 - 1, 0, screenWidth_ / 2 + 1, screenHeight_, GetColor(255, 255, 255), TRUE);
	DrawBox(0, screenHeight_ / 2 - 1, screenWidth_, screenHeight_ / 2 + 1, GetColor(255, 255, 255), TRUE);
}

void GameScene::CaptureScreenshot(void)
{
	if (sceneScreenHandle_ == -1 || screenshotScreenHandle_ == -1) { isScreenshotRequested_ = false; return; }

	SetDrawScreen(screenshotScreenHandle_); SetDrawArea(0, 0, screenWidth_, screenHeight_); ClearDrawScreen();
	DrawGraph(0, 0, sceneScreenHandle_, FALSE);

	hasScreenshot_ = true; isScreenshotRequested_ = false;
}

void GameScene::DrawScreenshotThumbnail(void) const
{
	if (!hasScreenshot_ || screenshotScreenHandle_ == -1) return;

	const int thumbnailRight = screenWidth_ - THUMBNAIL_MARGIN;
	const int thumbnailLeft = thumbnailRight - THUMBNAIL_WIDTH;
	const int thumbnailTop = THUMBNAIL_MARGIN + THUMBNAIL_LABEL_HEIGHT;
	const int thumbnailBottom = thumbnailTop + THUMBNAIL_HEIGHT;
	const int frameColor = GetColor(255, 255, 255);
	const int backColor = GetColor(0, 0, 0);
	const int labelColor = GetColor(255, 255, 0);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	DrawBox(thumbnailLeft - THUMBNAIL_FRAME_THICKNESS, THUMBNAIL_MARGIN - THUMBNAIL_FRAME_THICKNESS, thumbnailRight + THUMBNAIL_FRAME_THICKNESS, thumbnailBottom + THUMBNAIL_FRAME_THICKNESS, backColor, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	DrawBox(thumbnailLeft - THUMBNAIL_FRAME_THICKNESS, thumbnailTop - THUMBNAIL_FRAME_THICKNESS, thumbnailRight + THUMBNAIL_FRAME_THICKNESS, thumbnailBottom + THUMBNAIL_FRAME_THICKNESS, frameColor, TRUE);
	DrawExtendGraph(thumbnailLeft, thumbnailTop, thumbnailRight, thumbnailBottom, screenshotScreenHandle_, FALSE);
	DrawString(thumbnailLeft, THUMBNAIL_MARGIN, "LAST SHOT", labelColor);
}

void GameScene::DrawFlashEffect(void) const
{
	if (flashFrame_ <= 0) return;
	const int alpha = 255 * flashFrame_ / FLASH_FRAME_MAX;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawBox(0, 0, screenWidth_, screenHeight_, GetColor(255, 255, 255), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void GameScene::DrawGoalMarker(void) const
{
	// ゴールを示す簡易マーカー: 上に浮いた球と立てたポール
	const VECTOR spherePos = VAdd(GOAL_POS, VGet(0.0f, 45.0f, 0.0f));
	const VECTOR poleTop = VAdd(GOAL_POS, VGet(0.0f, 180.0f, 0.0f));
	const int ringColor = GetColor(0, 255, 120);
	const int innerColor = GetColor(200, 255, 220);

	// 透明度を弱めた円球（輪）
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	DrawSphere3D(spherePos, GOAL_RADIUS, 16, ringColor, innerColor, FALSE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// ポール
	DrawLine3D(GOAL_POS, poleTop, ringColor);

	// 小さな回転するリング表現を簡易に複数描画（視認性向上）
	for (int i = 0; i < 3; ++i)
	{
		float h = 20.0f + i * 10.0f;
		VECTOR ringPos = VAdd(GOAL_POS, VGet(0.0f, h, 0.0f));
		int c = GetColor(0, 200 - i * 40, 140 + i * 40);
		DrawSphere3D(ringPos, GOAL_RADIUS * (0.6f + 0.2f * i), 12, c, c, FALSE);
	}
}

bool GameScene::IsSubjectInView(const Player* targetPlayer, const Subject* targetSubject) const
{
	if (!targetPlayer || !targetSubject) return false;

	const VECTOR cameraPos = targetPlayer->GetCameraWorldPos();
	const VECTOR subjectHeadPos = VAdd(targetSubject->GetTransform().pos, Subject::COL_CAPSULE_TOP_LOCAL_POS);

	const VECTOR toSubject = VSub(subjectHeadPos, cameraPos);
	const float distance = VSize(toSubject);
	if (distance <= 0.0001f) return true;

	const VECTOR subjectDir = VScale(toSubject, 1.0f / distance);
	const VECTOR cameraForward = targetPlayer->GetCameraForward();

	const float dot = cameraForward.x * subjectDir.x + cameraForward.y * subjectDir.y + cameraForward.z * subjectDir.z;
	if (dot < PHOTO_SCORE_VIEW_DOT_MIN) return false;

	return IsSubjectVisible(targetPlayer, targetSubject);
}

int GameScene::CalculatePhotoScore(const VECTOR& shotPos, const VECTOR& targetPos) const
{
	const float distance = VSize(VSub(targetPos, shotPos));
	if (distance <= PHOTO_SCORE_NEAR_DISTANCE) return PHOTO_SCORE_MAX;
	if (distance >= PHOTO_SCORE_FAR_DISTANCE) return PHOTO_SCORE_MIN;

	const float t = (distance - PHOTO_SCORE_NEAR_DISTANCE) / (PHOTO_SCORE_FAR_DISTANCE - PHOTO_SCORE_NEAR_DISTANCE);
	int score = static_cast<int>(PHOTO_SCORE_MAX - (PHOTO_SCORE_MAX - PHOTO_SCORE_MIN) * t);
	if (score < PHOTO_SCORE_MIN) score = PHOTO_SCORE_MIN;
	if (score > PHOTO_SCORE_MAX) score = PHOTO_SCORE_MAX;
	return score;
}

void GameScene::TryTakePhoto()
{
	if (players_.empty() || subjectManager_ == nullptr) return;
	const auto& subjects = subjectManager_->GetSubjects();
	if (subjects.empty()) return;

	int totalAddedScore = 0;
	for (size_t i = 0; i < players_.size(); ++i)
	{
		Player* p = players_[i];
		if (!p || !IsPlayerAlive(p)) continue;
		int addScore = 0;
		const VECTOR shotPos = p->GetCameraWorldPos();
		for (const auto* subject : subjects)
		{
			if (!subject) continue;
			if (!IsSubjectInView(p, subject)) continue;
			addScore += CalculatePhotoScore(shotPos, subject->GetTransform().pos);
		}
		lastPhotoScorePerPlayer_[i] = addScore;
		if (addScore > 0) { photoCountPerPlayer_[i] += 1; totalAddedScore += addScore; }
	}

	if (totalAddedScore > 0)
	{
		lastPhotoScore_ = totalAddedScore;
		photoCount_ = 0; for (int v : photoCountPerPlayer_) photoCount_ += v;
		SceneManager& scene = SceneManager::GetInstance();
		scene.SetCarryMoney(scene.GetCarryMoney() + totalAddedScore);
	}
}

void GameScene::DrawSubjectDistanceGuide(const Player* targetPlayer) const
{
	if (!targetPlayer || !subjectManager_) return;
	const auto& subjects = subjectManager_->GetSubjects();
	if (subjects.empty()) return;

	const VECTOR playerHeadPos = VAdd(targetPlayer->GetTransform().pos, Player::COL_CAPSULE_TOP_LOCAL_POS);
	const int visibleLineColor = GetColor(255, 0, 0);
	const int hiddenLineColor = GetColor(0, 0, 255);
	const int textColor = GetColor(255, 255, 0);

	for (const auto* subject : subjects)
	{
		if (!subject) continue;
		const VECTOR subjectHeadPos = VAdd(subject->GetTransform().pos, Subject::COL_CAPSULE_TOP_LOCAL_POS);
		const float distance = VSize(VSub(subjectHeadPos, playerHeadPos));
		const bool isVisible = IsSubjectVisible(targetPlayer, subject);
		const int lineColor = isVisible ? visibleLineColor : hiddenLineColor;
		DrawLine3D(playerHeadPos, subjectHeadPos, lineColor);

		const VECTOR midPos = VScale(VAdd(playerHeadPos, subjectHeadPos), 0.5f);
		const VECTOR screenPos = ConvWorldPosToScreenPos(midPos);
		DrawFormatString(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y), textColor, "%.0f", distance);
	}
}

bool GameScene::IsSubjectVisible(const Player* targetPlayer, const Subject* targetSubject) const
{
	if (!targetPlayer || !targetSubject || !stage_) return false;

	const ColliderBase* stageColliderBase = stage_->GetOwnCollider(static_cast<int>(Stage::COLLIDER_TYPE::MODEL));
	if (!stageColliderBase || stageColliderBase->GetShape() != ColliderBase::SHAPE::MODEL) return true;

	const auto* stageCollider = static_cast<const ColliderModel*>(stageColliderBase);
	const VECTOR cameraPos = targetPlayer->GetCameraWorldPos();
	const VECTOR subjectHeadPos = VAdd(targetSubject->GetTransform().pos, Subject::COL_CAPSULE_TOP_LOCAL_POS);

	auto hit = stageCollider->GetNearestHitPolyLine(cameraPos, subjectHeadPos, true);
	if (!hit.HitFlag) return true;

	const float hitDistance = VSize(VSub(hit.HitPosition, cameraPos));
	const float subjectDistance = VSize(VSub(subjectHeadPos, cameraPos));
	return hitDistance >= subjectDistance - 1.0f;
}

bool GameScene::IsCameraOccludedByStage(const Player* targetPlayer) const
{
	if (!targetPlayer || !stage_) return false;

	const ColliderBase* stageColliderBase = stage_->GetOwnCollider(static_cast<int>(Stage::COLLIDER_TYPE::MODEL));
	if (!stageColliderBase || stageColliderBase->GetShape() != ColliderBase::SHAPE::MODEL) return false;

	const auto* stageCollider = static_cast<const ColliderModel*>(stageColliderBase);
	const VECTOR focusPos = VAdd(targetPlayer->GetTransform().pos, Player::COL_CAPSULE_TOP_LOCAL_POS);
	const VECTOR cameraPos = targetPlayer->GetCameraWorldPos();

	auto hit = stageCollider->GetNearestHitPolyLine(focusPos, cameraPos, true);
	if (!hit.HitFlag) return false;

	const float hitDistance = VSize(VSub(hit.HitPosition, focusPos));
	const float cameraDistance = VSize(VSub(cameraPos, focusPos));
	return hitDistance < cameraDistance - CAMERA_OCCLUDE_EPSILON;
}

void GameScene::ApplyStageOpacityForCamera(const Player* targetPlayer)
{
	if (!stage_) return;
	if (IsCameraOccludedByStage(targetPlayer)) stage_->SetOpacityRate(CAMERA_OCCLUDED_OPACITY);
	else stage_->SetOpacityRate(1.0f);
}

void GameScene::UpdateSubjectAttacks(void)
{
	if (!subjectManager_) return;
	const auto& subjects = subjectManager_->GetSubjects();
	if (subjects.empty()) return;

	std::vector<Player*> players;
	if (player_) players.push_back(player_);
	if (player2_) players.push_back(player2_);
	if (player3_) players.push_back(player3_);
	if (player4_) players.push_back(player4_);

	for (auto* subject : subjects)
	{
		if (!subject) continue;
		Player* nearest = nullptr; float nearestDist = FLT_MAX;
		for (auto* p : players)
		{
			if (!p) continue;
			const VECTOR pPos = p->GetTransform().pos;
			const float d = VSize(VSub(pPos, subject->GetTransform().pos));
			if (d < nearestDist) { nearestDist = d; nearest = p; }
		}
		if (!nearest) continue;
		const VECTOR nearestPos = nearest->GetTransform().pos;
		if (subject->CanStartAttack() && subject->IsInAttackRange(nearestPos)) subject->StartAttack(nearestPos);
		if (subject->ConsumeAttackHit() && subject->IsInAttackRange(nearestPos)) nearest->TakeDamage(1);
	}
}

bool GameScene::IsPlayerAlive(const Player* targetPlayer) const { return targetPlayer != nullptr && !targetPlayer->IsDead(); }

bool GameScene::IsPlayerAtGoal(const Player* targetPlayer) const
{
	if (!targetPlayer) return false;
	VECTOR diff = VSub(targetPlayer->GetTransform().pos, GOAL_POS); diff.y = 0.0f;
	return VSize(diff) <= GOAL_RADIUS;
}

bool GameScene::IsPlayerReachedGoal(void) const
{
	bool hasAlive = false;
	for (const auto* p : players_)
	{
		if (!IsPlayerAlive(p)) continue;
		hasAlive = true;
		if (!IsPlayerAtGoal(p)) return false;
	}
	return hasAlive;
}

bool GameScene::IsAllPlayersDead(void) const
{
	if (players_.empty()) return false;
	for (const auto* p : players_) if (IsPlayerAlive(p)) return false;
	return true;
}