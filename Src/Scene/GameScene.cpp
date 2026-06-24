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
#include "../Utility/AsoUtility.h"
#include"../Manager/EffectManager.h"
#include <EffekseerForDXLib.h>

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

//====================================================
// GameScene初期化
//
// ・ステージ生成
// ・プレイヤー生成
// ・アイテム配布
// ・サブジェクト生成
// ・分割画面生成
// ・カメラ初期設定
//====================================================
void GameScene::Init()
{
	SceneManager& scene = SceneManager::GetInstance();

	stage_ = new Stage();
	stage_->Init();

	effectManager_ =std::make_unique<EffectManager>();
	effectManager_->Init();

	const ColliderBase* stageCollider =
		stage_->GetOwnCollider(static_cast<int>(Stage::COLLIDER_TYPE::MODEL));

	

	// プレイヤー選択情報を取得して GameScene の構成に反映
	const int selectedPlayerCount = scene.GetPlayerNum();
	const auto& selectedPlayers = scene.GetSelectedPlayerNums();
	activePlayerCount_ = selectedPlayerCount;

	SetupPlayers(stageCollider, selectedPlayerCount);
	RebuildPlayersArray();

	// プレイヤー1（常に作成）
	player_ = new Player();
	player_->Init();
	player_->SetInputConfig(Player::KEYBOARD_AND_PAD1_INPUT_CONFIG);
	player_->SetInputEnabled(true);
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
		subjectManager_->CreateRandomSubject();
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

	isSplitScreenEnabled_ = (selectedPlayerCount > 1);

	GetDrawScreenSize(&screenWidth_, &screenHeight_);
	CreateScreenHandles(selectedPlayerCount);

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
	if (player_ != nullptr)
	{
		camera->SetAngles(player_->GetCameraAngles());
	}
	camera->ChangeMode(Camera::MODE::FREE);
}


//====================================================
// 毎フレーム更新
//
// ・プレイヤー更新
// ・サブジェクト更新
// ・アイテム使用処理
// ・トラップ判定
// ・ゲームクリア判定
// ・ゲームオーバー判定
// ・撮影処理
//====================================================
void GameScene::Update()
{
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	const auto padNo = InputManager::JOYPAD_NO::PAD1;

	const bool isCycleItem =
		ins.IsTrgDown(KEY_INPUT_TAB);

	const bool isUseItem =
		ins.IsTrgDown(KEY_INPUT_E) ||
		ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::R_TRIGGER);

	const bool isTakePhoto =
		ins.IsTrgDown(KEY_INPUT_RETURN) ||
		ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::L_TRIGGER);

	const bool isTogglePreview =
		ins.IsTrgDown(KEY_INPUT_F1);

	if (flashFrame_ > 0)
	{
		flashFrame_--;
	}

	if (stage_) stage_->Update();

	UpdatePlayers();

	if (subjectManager_ != nullptr)
	{
		subjectManager_->Update();
	}

	if (isCycleItem && player_ != nullptr)
	{
		player_->CycleSelectedUsableItem(1);
	}

	if (isUseItem && player_ != nullptr)
	{
		ITEM_TYPE currentSel = player_->GetSelectedUsableItemType();
		if (currentSel == ITEM_TYPE::NORMAL_CAMERA)
		{
			player_->CycleSelectedUsableItem(0);
		}

		const ITEM_TYPE sel = player_->GetSelectedUsableItemType();

		if (sel != ITEM_TYPE::NORMAL_CAMERA)
		{
			const VECTOR ppos = player_->GetTransform().pos;

			VECTOR placeForward =
				player_->GetCameraForward();

			placeForward.y = 0.0f;

			float len = sqrtf(
				placeForward.x * placeForward.x +
				placeForward.z * placeForward.z);

			if (len > 0.0f)
			{
				placeForward.x /= len;
				placeForward.z /= len;
			}

			VECTOR placePos =
				VAdd(
					ppos,
					VScale(placeForward, 140.0f));

			placePos.y = ppos.y;

			switch (sel)
			{
			case ITEM_TYPE::SPIKE_TRAP:
				if (player_->UseSpikeTrap())
				{
					Trap t;
					t.type = TRAP_TYPE::SPIKE;
					t.pos = placePos;

					t.modelId =
						ResourceManager::GetInstance().
						LoadModelDuplicate(
							ResourceManager::SRC::SPIKE_MODEL);

					MV1SetPosition(
						t.modelId,
						t.pos);

					MV1SetScale(
						t.modelId,
						VGet(0.01f, 0.01f, 0.01f));

					t.triggered = false;
					t.lifeFrames = SPIKE_DURATION_FRAMES;

					traps_.push_back(t);

					if (gs_placeSE != -1)
					{
						PlaySoundMem(
							gs_placeSE,
							DX_PLAYTYPE_BACK);
					}

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
					t.pos = placePos;
					t.triggered = false;
					t.lifeFrames = 0;

					t.modelId =
						ResourceManager::GetInstance().
						LoadModelDuplicate(
							ResourceManager::SRC::MINE_MODEL);

					MV1SetPosition(
						t.modelId,
						t.pos);

					MV1SetScale(
						t.modelId,
						VGet(1.0f, 1.0f, 1.0f));

					traps_.push_back(t);

					if (gs_placeSE != -1)
					{
						PlaySoundMem(
							gs_placeSE,
							DX_PLAYTYPE_BACK);
					}

					if (player_->GetMineCount() <= 0)
					{
						player_->CycleSelectedUsableItem(1);
					}
				}
				break;

			case ITEM_TYPE::FRAG_GRENADE:
				if (player_->UseFragGrenade())
				{
					Grenade g;

					VECTOR forward =
						player_->GetCameraForward();

					g.pos =
						player_->GetTransform().pos;

					g.pos.y += 60.0f;

					g.pos = VAdd(
						g.pos,
						VScale(forward, 30.0f));

					g.velocity = VAdd(
						VScale(forward, 15.0f),
						VGet(0.0f, 10.0f, 0.0f));

					g.exploded = false;
					g.lifeFrame = 0;

					grenades_.push_back(g);

					if (gs_placeSE != -1)
					{
						PlaySoundMem(
							gs_placeSE,
							DX_PLAYTYPE_BACK);
					}

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
			else
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
							if (!s->IsDying())
							{
								VECTOR dir =
									AsoUtility::VNormalize(diff);

								s->AddKnockBack(
									VScale(dir, 10.0f));

								s->StartDying();
							}

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

							VECTOR effectPos = trap.pos;
							effectPos.y += 50.0f;

							effectManager_->AddExplosion(
								effectPos);

							effectManager_->PlayExplosion(
								effectPos);

							trap.triggered = true;
							trap.lifeFrames = 30;

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

		traps_.erase(
			std::remove_if(traps_.begin(), traps_.end(), [](const Trap& t) { return t.triggered && t.lifeFrames <= 0; }),
			traps_.end());
	}

	// 追加: グレネードの更新
	for (auto it = grenades_.begin();
		it != grenades_.end();)
	{
		Grenade& g = *it;

		if (!g.exploded)
		{
			g.velocity.y -= 0.4f;
			g.pos = VAdd(g.pos, g.velocity);

			if (g.pos.y <= 0.0f)
			{
				g.pos.y = 0.0f;
				ExplodeGrenade(g.pos);
				g.exploded = true;
				g.lifeFrame = 0;
			}
		}
		else
		{
			g.lifeFrame++;

			if (g.lifeFrame > 30)
			{
				it = grenades_.erase(it);
				continue;
			}
		}

		++it;
	}

	effectManager_->Update();

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

	if (isTakePhoto)
	{
		TryTakePhoto();
		isScreenshotRequested_ = true;
		flashFrame_ = FLASH_FRAME_MAX;
	}

	if (isTogglePreview && hasScreenshot_)
	{
		isScreenshotPreviewEnabled_ = !isScreenshotPreviewEnabled_;
	}
}

//====================================================
// 描画処理
//
// ・各プレイヤー視点描画
// ・スクリーンショット描画
// ・フラッシュ演出描画
//====================================================
void GameScene::Draw()
{
	DrawCompositedScene();

	DrawFormatString(
		0,
		0,
		GetColor(255, 255, 255),
		"GAME DRAW");

	if (isScreenshotRequested_)
	{
		CaptureScreenshot();
	}

	SetDrawScreen(DX_SCREEN_BACK);
	SetDrawArea(0, 0, screenWidth_, screenHeight_);
	ClearDrawScreen();

	DrawGraph(0, 0, sceneScreenHandle_, FALSE);

	if (hasScreenshot_)
	{
		if (isScreenshotPreviewEnabled_)
		{
			DrawScreenshotPreview();
			DrawString(20, 140, "F1 : CLOSE SCREENSHOT", GetColor(255, 255, 255));
		}
		else
		{
			DrawScreenshotThumbnail();
			DrawString(20, 140, "F1 : OPEN SCREENSHOT", GetColor(255, 255, 255));
		}
	}

	int x = 300;
	int y = 40;
	int color = GetColor(255, 255, 255);

	DrawString(x, y, "【操作方法】", color);

	y += 40;
	DrawString(x, y, "移動　　　　：左スティック", color);

	y += 30;
	DrawString(x, y, "カメラ操作　：右スティック", color);

	y += 40;
	DrawString(x, y, "LT　　　　　：写真撮影", color);

	y += 30;
	DrawString(x, y, "RT　　　　　：アイテム投下", color);

	

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

void GameScene::ExplodeGrenade(const VECTOR& pos)
{
	VECTOR effectPos = pos;
	effectPos.y += 50.0f;

	effectManager_->AddExplosion(
		effectPos);

	effectManager_->PlayExplosion(
		pos);

	const float radius = 200.0f;

	for (auto* s : subjectManager_->GetSubjects())
	{
		if (s == nullptr)
		{
			continue;
		}

		VECTOR diff =
			VSub(s->GetTransform().pos, pos);

		diff.y = 0.0f;

		float dist = VSize(diff);

		if (VSize(diff) <= radius)
		{
			VECTOR dir =
				AsoUtility::VNormalize(diff);

			s->AddKnockBack(
				VScale(dir, 12.0f));

			if (!s->IsDying())
			{
				s->StartDying();
			}
		}
	}

	for (auto* pl : players_)
	{
		if (pl == nullptr)
		{
			continue;
		}

		VECTOR diff =
			VSub(pl->GetTransform().pos, pos);

		diff.y = 0.0f;

		if (VSize(diff) <= radius)
		{
			pl->TakeDamage(
				std::max(1, pl->GetHpMax() / 3));
		}
	}
}

void GameScene::Draw3D()
{
	// 3D描画が必要な場合に処理を追加
}

void GameScene::Release()
{
	ReleasePlayers();


	
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

	for (auto& trap : traps_)
	{
		if (trap.modelId != -1)
		{
			MV1DeleteModel(trap.modelId);
			trap.modelId = -1;
		}
	}

	traps_.clear();

	traps_.clear();

	ReleaseScreenHandles();
}

Player* GameScene::CreatePlayer(
	const ColliderBase* stageCollider,
	const VECTOR* initPos,
	bool usePlayer2InputConfig,
	bool enableInput)
{
	Player* player = new Player();
	player->Init();

	if (usePlayer2InputConfig)
	{
		player->SetInputConfig(Player::PLAYER2_KEYBOARD_INPUT_CONFIG);
	}

	if (enableInput)
	{
		player->SetInputEnabled(true);
	}

	if (initPos != nullptr)
	{
		player->SetPos(*initPos);
		player->SetCameraAngles(VGet(0.0f, DX_PI_F, 0.0f));
	}

	player->AddHitCollider(stageCollider);

	return player;
}

void GameScene::RebuildPlayersArray(void)
{
	players_.clear();

	if (player_) players_.push_back(player_);
	if (player2_) players_.push_back(player2_);
	if (player3_) players_.push_back(player3_);
	if (player4_) players_.push_back(player4_);

	const size_t pcount = players_.size();
	lastPhotoScorePerPlayer_.assign(pcount, 0);
	photoCountPerPlayer_.assign(pcount, 0);
}

void GameScene::UpdatePlayers(void)
{
	for (auto* player : players_)
	{
		if (player == nullptr)
		{
			continue;
		}

		player->Update();
	}
}

void GameScene::ReleasePlayers(void)
{
	for (auto*& player : players_)
	{
		if (player == nullptr)
		{
			continue;
		}

		player->Release();
		delete player;
		player = nullptr;
	}

	players_.clear();
	lastPhotoScorePerPlayer_.clear();
	photoCountPerPlayer_.clear();

	player_ = nullptr;
	player2_ = nullptr;
	player3_ = nullptr;
	player4_ = nullptr;
}

void GameScene::DeleteScreenHandle(int& screenHandle)
{
	if (screenHandle == -1)
	{
		return;
	}

	DeleteGraph(screenHandle);
	screenHandle = -1;
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

	SetDrawScreen(screenHandle);
	SetDrawArea(0, 0, drawWidth, drawHeight);
	ClearDrawScreen();


	//----------------------------------------------------
	// カメラ設定
	//----------------------------------------------------
	targetPlayer->ApplyCamera(SceneManager::GetInstance().GetCamera());

	VECTOR eye = GetCameraPosition();
	VECTOR target = GetCameraTarget();

	DrawLine3D(
		eye,
		target,
		GetColor(255, 0, 0));

	//----------------------------------------------------
	// ワールド描画
	// ・ステージ
	// ・サブジェクト
	// ・プレイヤー
	//----------------------------------------------------

	VECTOR camPos = GetCameraPosition();


	DrawViewWorld(targetPlayer, hidePlayer);

	

	//----------------------------------------------------
	// HUD描画
	//----------------------------------------------------
	DrawViewHud(targetPlayer, playerName, drawWidth);

	// トラップ描画
	for (const auto& trap : traps_)
	{

		// 地雷はモデル描画
		if (trap.type == TRAP_TYPE::MINE)
		{

			VECTOR pos = MV1GetPosition(trap.modelId);

			if (trap.modelId != -1)
			{
				MV1DrawModel(trap.modelId);
			}

			continue;
		}

		effectManager_->Draw();

		// ===== SPIKE描画 =====

		if (trap.modelId != -1)
		{
			MV1DrawModel(trap.modelId);
			continue;
		}

		// ワイヤー表示
		const int outline =
			GetColor(255, 255, 255);

		const int ringOutline =
			GetColor(255, 220, 120);

		
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

void GameScene::DrawCompositedScene(void)
{
	if (sceneScreenHandle_ == -1)
	{
		return;
	}

	if (!isSplitScreenEnabled_ || activePlayerCount_ <= 1)
	{
		DrawSinglePlayerScene();
		effectManager_->Draw();
		return;
	}

	if (activePlayerCount_ == 2)
	{
		DrawTwoPlayerScene();
		effectManager_->Draw();
		return;
	}

	DrawFourPlayerScene();
	effectManager_->Draw();
}

void GameScene::CaptureScreenshot(void)
{
	if (screenshotScreenHandle_ == -1)
	{
		isScreenshotRequested_ = false;
		return;
	}

	int sourceHandle = -1;
	int sourceWidth = screenWidth_;
	int sourceHeight = screenHeight_;

	if (!isSplitScreenEnabled_ || activePlayerCount_ <= 1)
	{
		sourceHandle = sceneScreenHandle_;
	}
	else if (activePlayerCount_ == 2)
	{
		sourceHandle = leftScreenHandle_;
		sourceWidth = screenWidth_ / 2;
		sourceHeight = screenHeight_;
	}
	else
	{
		sourceHandle = leftScreenHandle_;
		sourceWidth = screenWidth_ / 2;
		sourceHeight = screenHeight_ / 2;
	}

	if (sourceHandle == -1)
	{
		isScreenshotRequested_ = false;
		return;
	}

	SetDrawScreen(screenshotScreenHandle_);
	SetDrawArea(0, 0, screenWidth_, screenHeight_);
	ClearDrawScreen();

	DrawExtendGraph(
		0,
		0,
		screenWidth_,
		screenHeight_,
		sourceHandle,
		FALSE);

	hasScreenshot_ = true;
	isScreenshotRequested_ = false;
}

void GameScene::DrawScreenshotThumbnail(void) const
{
	if (!hasScreenshot_ || screenshotScreenHandle_ == -1)
	{
		return;
	}

	int areaX = 0;
	int areaY = 0;
	int areaWidth = 0;
	int areaHeight = 0;
	GetPlayer1ViewArea(areaX, areaY, areaWidth, areaHeight);

	const int maxWidth = areaWidth - THUMBNAIL_MARGIN * 2;
	const int maxHeight = areaHeight - THUMBNAIL_MARGIN * 2 - THUMBNAIL_LABEL_HEIGHT;

	if (maxWidth <= 0 || maxHeight <= 0)
	{
		return;
	}

	float scale = 1.0f;
	if (THUMBNAIL_WIDTH > maxWidth || THUMBNAIL_HEIGHT > maxHeight)
	{
		const float scaleX = static_cast<float>(maxWidth) / static_cast<float>(THUMBNAIL_WIDTH);
		const float scaleY = static_cast<float>(maxHeight) / static_cast<float>(THUMBNAIL_HEIGHT);
		scale = (scaleX < scaleY) ? scaleX : scaleY;
	}

	const int thumbnailWidth = static_cast<int>(THUMBNAIL_WIDTH * scale);
	const int thumbnailHeight = static_cast<int>(THUMBNAIL_HEIGHT * scale);

	const int thumbnailRight = areaX + areaWidth - THUMBNAIL_MARGIN;
	const int thumbnailLeft = thumbnailRight - thumbnailWidth;
	const int thumbnailTop = areaY + THUMBNAIL_MARGIN + THUMBNAIL_LABEL_HEIGHT;
	const int thumbnailBottom = thumbnailTop + thumbnailHeight;

	const int frameColor = GetColor(255, 255, 255);
	const int backColor = GetColor(0, 0, 0);
	const int labelColor = GetColor(255, 255, 0);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	DrawBox(
		thumbnailLeft - THUMBNAIL_FRAME_THICKNESS,
		areaY + THUMBNAIL_MARGIN - THUMBNAIL_FRAME_THICKNESS,
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
		FALSE);

	DrawExtendGraph(
		thumbnailLeft,
		thumbnailTop,
		thumbnailRight,
		thumbnailBottom,
		screenshotScreenHandle_,
		FALSE);

	DrawString(thumbnailLeft, areaY + THUMBNAIL_MARGIN, "LAST SHOT", labelColor);
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

int GameScene::CalculatePlayerPhotoScore(const Player* targetPlayer) const
{
	if (targetPlayer == nullptr || subjectManager_ == nullptr)
	{
		return 0;
	}

	const auto& subjects = subjectManager_->GetSubjects();
	if (subjects.empty())
	{
		return 0;
	}

	int addScore = 0;
	const VECTOR shotPos = targetPlayer->GetCameraWorldPos();

	for (const auto* subject : subjects)
	{
		if (subject == nullptr)
		{
			continue;
		}

		if (!IsSubjectInView(targetPlayer, subject))
		{
			continue;
		}

		addScore += CalculatePhotoScore(shotPos, subject->GetTransform().pos);
	}

	return addScore;
}

void GameScene::TryTakePhoto(void)
{
	if (players_.empty() || subjectManager_ == nullptr)
	{
		return;
	}

	const auto& subjects = subjectManager_->GetSubjects();
	if (subjects.empty())
	{
		return;
	}

	int totalAddedScore = 0;

	for (size_t i = 0; i < players_.size(); ++i)
	{
		Player* player = players_[i];
		if (player == nullptr || !IsPlayerAlive(player))
		{
			continue;
		}

		const int addScore = CalculatePlayerPhotoScore(player);
		lastPhotoScorePerPlayer_[i] = addScore;

		if (addScore > 0)
		{
			photoCountPerPlayer_[i] += 1;
			totalAddedScore += addScore;
		}
	}

	ApplyPhotoScoreResult(totalAddedScore);
}

void GameScene::ApplyPhotoScoreResult(int totalAddedScore)
{
	if (totalAddedScore <= 0)
	{
		return;
	}

	lastPhotoScore_ = totalAddedScore;
	photoCount_ = 0;

	for (int count : photoCountPerPlayer_)
	{
		photoCount_ += count;
	}

	SceneManager& scene = SceneManager::GetInstance();
	scene.SetCarryMoney(scene.GetCarryMoney() + totalAddedScore);
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

	const VECTOR playerHeadPos = targetPlayer->GetHeadWorldPos();

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

	const VECTOR cameraPos = targetPlayer->GetCameraWorldPos();
	const VECTOR subjectHeadPos = VAdd(
		targetSubject->GetTransform().pos,
		Subject::COL_CAPSULE_TOP_LOCAL_POS);

	return stage_->HasLineOfSight(cameraPos, subjectHeadPos, 1.0f);
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
	if (subjectManager_ == nullptr)
	{
		return;
	}

	const auto& subjects = subjectManager_->GetSubjects();
	if (subjects.empty() || players_.empty())
	{
		return;
	}

	for (auto* subject : subjects)
	{
		if (subject == nullptr)
		{
			continue;
		}

		Player* nearest = nullptr;
		float nearestDist = FLT_MAX;

		for (auto* player : players_)
		{
			if (player == nullptr)
			{
				continue;
			}

			const VECTOR playerPos = player->GetTransform().pos;
			const float distance = VSize(VSub(playerPos, subject->GetTransform().pos));
			if (distance < nearestDist)
			{
				nearestDist = distance;
				nearest = player;
			}
		}

		if (nearest == nullptr)
		{
			continue;
		}

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

bool GameScene::IsPlayerAlive(const Player* targetPlayer) const { return targetPlayer != nullptr && !targetPlayer->IsDead(); }

bool GameScene::IsPlayerAtGoal(const Player* targetPlayer) const
{
	if (!targetPlayer) return false;
	VECTOR diff = VSub(targetPlayer->GetTransform().pos, GOAL_POS); diff.y = 0.0f;
	return VSize(diff) <= GOAL_RADIUS;
}

bool GameScene::IsPlayerReachedGoal(void) const
{
	if (stage_ == nullptr)
	{
		return false;
	}

	bool hasAlivePlayer = false;

	for (const auto* player : players_)
	{
		if (!IsPlayerAlive(player))
		{
			continue;
		}

		hasAlivePlayer = true;

		if (!stage_->IsAtGoal(player->GetTransform().pos))
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

const Player* GameScene::GetPlayerByIndex(int index) const
{
	if (index < 0 || index >= static_cast<int>(players_.size()))
	{
		return nullptr;
	}

	return players_[index];
}

void GameScene::DrawPlayers(const Player* hidePlayer)
{
	for (auto* player : players_)
	{
		if (player == nullptr || player == hidePlayer)
		{
			continue;
		}

		player->Draw();
	}
}

void GameScene::SetupPlayers(const ColliderBase* stageCollider, int selectedPlayerCount)
{
	ResetPlayerSlots();

	struct PlayerSetup
	{
		Player** player;
		const VECTOR* initPos;
		bool usePlayer2InputConfig;
		bool enableInput;
	};

	player_ = CreatePlayer(stageCollider);

	PlayerSetup setups[] =
	{
		{ &player2_, &PLAYER2_INIT_POS, true,  true  },
		{ &player3_, &PLAYER3_INIT_POS, false, true  },
		{ &player4_, &PLAYER4_INIT_POS, false, true  },
	};

	const int optionalPlayerCount = selectedPlayerCount - 1;
	const int setupCount = static_cast<int>(sizeof(setups) / sizeof(setups[0]));

	for (int i = 0; i < setupCount; i++)
	{
		if (i < optionalPlayerCount)
		{
			*setups[i].player = CreatePlayer(
				stageCollider,
				setups[i].initPos,
				setups[i].usePlayer2InputConfig,
				setups[i].enableInput);
		}
	}
}

void GameScene::ResetPlayerSlots(void)
{
	player_ = nullptr;
	player2_ = nullptr;
	player3_ = nullptr;
	player4_ = nullptr;
}

void GameScene::CreateScreenHandles(int selectedPlayerCount)
{
	ResetScreenHandles();

	if (isSplitScreenEnabled_ && selectedPlayerCount == 2)
	{
		leftScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_, TRUE);
		rightScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_, TRUE);
	}
	else if (isSplitScreenEnabled_ && selectedPlayerCount >= 3)
	{
		leftScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_ / 2, TRUE);
		rightScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_ / 2, TRUE);
		bottomLeftScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_ / 2, TRUE);
		bottomRightScreenHandle_ = MakeScreen(screenWidth_ / 2, screenHeight_ / 2, TRUE);
	}

	sceneScreenHandle_ = MakeScreen(screenWidth_, screenHeight_, TRUE);
	screenshotScreenHandle_ = MakeScreen(screenWidth_, screenHeight_, TRUE);
}

void GameScene::ReleaseScreenHandles(void)
{
	DeleteScreenHandle(leftScreenHandle_);
	DeleteScreenHandle(rightScreenHandle_);
	DeleteScreenHandle(bottomLeftScreenHandle_);
	DeleteScreenHandle(bottomRightScreenHandle_);
	DeleteScreenHandle(sceneScreenHandle_);
	DeleteScreenHandle(screenshotScreenHandle_);
}

void GameScene::ResetScreenHandles(void)
{
	leftScreenHandle_ = -1;
	rightScreenHandle_ = -1;
	bottomLeftScreenHandle_ = -1;
	bottomRightScreenHandle_ = -1;
	sceneScreenHandle_ = -1;
	screenshotScreenHandle_ = -1;
}

void GameScene::DrawSinglePlayerScene(void)
{
	static const char* PLAYER_NAMES[] =
	{
		"PLAYER 1",
		"PLAYER 2",
		"PLAYER 3",
		"PLAYER 4",
	};

	DrawView(
		sceneScreenHandle_,
		screenWidth_,
		screenHeight_,
		GetPlayerByIndex(0),
		nullptr,
		PLAYER_NAMES[0]);
}

void GameScene::DrawTwoPlayerScene(void)
{
	static const char* PLAYER_NAMES[] =
	{
		"PLAYER 1",
		"PLAYER 2",
		"PLAYER 3",
		"PLAYER 4",
	};

	DrawView(
		leftScreenHandle_,
		screenWidth_ / 2,
		screenHeight_,
		GetPlayerByIndex(0),
		nullptr,
		PLAYER_NAMES[0]);

	DrawView(
		rightScreenHandle_,
		screenWidth_ / 2,
		screenHeight_,
		GetPlayerByIndex(1),
		nullptr,
		PLAYER_NAMES[1]);

	ComposeSplitScreens(false);
}

void GameScene::DrawFourPlayerScene(void)
{
	static const char* PLAYER_NAMES[] =
	{
		"PLAYER 1",
		"PLAYER 2",
		"PLAYER 3",
		"PLAYER 4",
	};

	const Player* player1 = GetPlayerByIndex(0);
	const Player* player2 = GetPlayerByIndex(1);
	const Player* player3 = GetPlayerByIndex(2);
	const Player* player4 = GetPlayerByIndex(3);

	DrawView(
		leftScreenHandle_,
		screenWidth_ / 2,
		screenHeight_ / 2,
		player1,
		nullptr,
		PLAYER_NAMES[0]);

	DrawView(
		rightScreenHandle_,
		screenWidth_ / 2,
		screenHeight_ / 2,
		player2 ? player2 : player1,
		nullptr,
		PLAYER_NAMES[1]);

	DrawView(
		bottomLeftScreenHandle_,
		screenWidth_ / 2,
		screenHeight_ / 2,
		player3 ? player3 : player1,
		nullptr,
		PLAYER_NAMES[2]);

	if (player4 != nullptr)
	{
		DrawView(
			bottomRightScreenHandle_,
			screenWidth_ / 2,
			screenHeight_ / 2,
			player4,
			nullptr,
			PLAYER_NAMES[3]);
	}
	else
	{
		DrawEmptyView(
			bottomRightScreenHandle_,
			screenWidth_ / 2,
			screenHeight_ / 2);
	}

	ComposeSplitScreens(true);
}

void GameScene::ComposeSplitScreens(bool isFourWay)
{
	SetDrawScreen(sceneScreenHandle_);
	SetDrawArea(0, 0, screenWidth_, screenHeight_);
	ClearDrawScreen();

	DrawGraph(0, 0, leftScreenHandle_, FALSE);
	DrawGraph(screenWidth_ / 2, 0, rightScreenHandle_, FALSE);

	if (isFourWay)
	{
		DrawGraph(0, screenHeight_ / 2, bottomLeftScreenHandle_, FALSE);
		DrawGraph(screenWidth_ / 2, screenHeight_ / 2, bottomRightScreenHandle_, FALSE);

		DrawBox(
			0,
			screenHeight_ / 2 - 1,
			screenWidth_,
			screenHeight_ / 2 + 1,
			GetColor(255, 255, 255),
			TRUE);
	}

	DrawBox(
		screenWidth_ / 2 - 1,
		0,
		screenWidth_ / 2 + 1,
		screenHeight_,
		GetColor(255, 255, 255),
		TRUE);
}

void GameScene::DrawEmptyView(int screenHandle, int drawWidth, int drawHeight) const
{
	if (screenHandle == -1)
	{
		return;
	}

	SetDrawScreen(screenHandle);
	SetDrawArea(0, 0, drawWidth, drawHeight);
	ClearDrawScreen();

	DrawBox(0, 0, drawWidth, drawHeight, GetColor(10, 10, 10), TRUE);
	DrawFormatString(
		drawWidth / 2 - 40,
		drawHeight / 2 - 8,
		GetColor(180, 180, 180),
		"NO PLAYER");
}

void GameScene::DrawViewWorld(const Player* targetPlayer, const Player* hidePlayer)
{
	if (stage_ != nullptr && targetPlayer != nullptr)
	{
		const VECTOR focusPos = targetPlayer->GetHeadWorldPos();
		const VECTOR cameraPos = targetPlayer->GetCameraWorldPos();

		stage_->UpdateOpacityForSegment(
			focusPos,
			cameraPos,
			CAMERA_OCCLUDED_OPACITY,
			CAMERA_OCCLUDE_EPSILON);

		stage_->Draw();
		stage_->SetOpacityRate(1.0f);
		stage_->DrawGoalMarker();
	}

	if (subjectManager_ != nullptr)
	{
		subjectManager_->Draw();
	}

	// グレネード描画
	for (const auto& g : grenades_)
	{
		DrawSphere3D(
			g.pos,
			10.0f,
			8,
			GetColor(255, 255, 0),
			GetColor(255, 255, 0),
			TRUE);

		DrawSphere3D(
			g.pos,
			30.0f,
			8,
			GetColor(255, 0, 0),
			GetColor(255, 0, 0),
			FALSE);
	}

	effectManager_->Draw();

	DrawSubjectDistanceGuide(targetPlayer);
	DrawPlayers(hidePlayer);
}

void GameScene::DrawViewHud(const Player* targetPlayer, const char* playerName, int drawWidth) const
{
	DrawString(20, 20, playerName, GetColor(255, 255, 255));
	DrawFormatString(
		20,
		50,
		GetColor(255, 255, 0),
		"SCORE : %d",
		SceneManager::GetInstance().GetCarryMoney());

	DrawPlayerPhotoInfo(targetPlayer);
	DrawPlayerHpBar(targetPlayer, drawWidth);
}

void GameScene::DrawPlayerHpBar(const Player* targetPlayer, int drawWidth) const
{
	if (targetPlayer == nullptr)
	{
		return;
	}

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

void GameScene::DrawPlayerPhotoInfo(const Player* targetPlayer) const
{
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

void GameScene::DrawScreenshotPreview(void) const
{
	const int previewRight = screenWidth_ - THUMBNAIL_MARGIN;
	const int previewLeft = previewRight - PREVIEW_WIDTH;
	const int previewTop = THUMBNAIL_MARGIN;
	const int previewBottom = previewTop + PREVIEW_HEIGHT;
	const int frameColor = GetColor(255, 255, 255);
	const int backColor = GetColor(0, 0, 0);
	const int labelColor = GetColor(255, 255, 0);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
	DrawBox(
		previewLeft - THUMBNAIL_FRAME_THICKNESS,
		previewTop - THUMBNAIL_FRAME_THICKNESS,
		previewRight + THUMBNAIL_FRAME_THICKNESS,
		previewBottom + THUMBNAIL_FRAME_THICKNESS + THUMBNAIL_LABEL_HEIGHT,
		backColor,
		TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	DrawBox(
		previewLeft - THUMBNAIL_FRAME_THICKNESS,
		previewTop - THUMBNAIL_FRAME_THICKNESS,
		previewRight + THUMBNAIL_FRAME_THICKNESS,
		previewBottom + THUMBNAIL_FRAME_THICKNESS,
		frameColor,
		FALSE);

	DrawExtendGraph(
		previewLeft,
		previewTop,
		previewRight,
		previewBottom,
		screenshotScreenHandle_,
		FALSE);

	DrawString(previewLeft, previewBottom + 8, "F1 : CLOSE PREVIEW", labelColor);
}

void GameScene::GetPlayer1ViewArea(int& x, int& y, int& width, int& height) const
{
	x = 0;
	y = 0;
	width = screenWidth_;
	height = screenHeight_;

	if (!isSplitScreenEnabled_ || activePlayerCount_ <= 1)
	{
		return;
	}

	if (activePlayerCount_ == 2)
	{
		width = screenWidth_ / 2;
		height = screenHeight_;
		return;
	}

	width = screenWidth_ / 2;
	height = screenHeight_ / 2;
}


