#include <cmath>
#include "PlayerNumScene.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"

PlayerNumScene::PlayerNumScene(void)
{
}

PlayerNumScene::~PlayerNumScene(void)
{
}

void PlayerNumScene::Init(void)
{

	selectImg_[0] = LoadGraph("Data/PlayerNum/1P.png");
	selectImg_[1] = LoadGraph("Data/PlayerNum/2P.png");
	selectImg_[2] = LoadGraph("Data/PlayerNum/3P.png");
	selectImg_[3] = LoadGraph("Data/PlayerNum/4P.png");
	useImg_		= LoadGraph("Data/PlayerNum/select.png");
	notUseImg_ = LoadGraph("Data/PlayerNum/select2.png");
	bgImg_ = LoadGraph("Data/PlayerNum/bg.jpg");

	decideSE_ = LoadSoundMem("Data/Sound/decide.wav");

	selectNum_ = 0;

	cursor_ = 0;

	for (int i = 0; i < 4; i++)
	{
		isUsePlayer_[i] = false;
	}

	// 最初は1PだけON
	isUsePlayer_[0] = true;

	/*printfDx("bgImg = %d\n", bgImg_);*/
}

void PlayerNumScene::Update(void)
{
	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	// Enterキーでゲームシーンへ遷移
	if (ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		// 選択中プレイヤー情報を SceneManager に保存
		std::vector<int> selected;
		for (int i = 0; i < SELECT_MAX; ++i)
		{
			if (isUsePlayer_[i])
			{
				// プレイヤー番号は 1 始まりで渡す (必要に応じて変更)
				selected.push_back(i + 1);
			}
		}

		// 選択人数を保存（selected.size() が 0 になる可能性は低いが安全策として 1 を保証）
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

		scene.ChangeScene(SceneManager::SCENE_ID::EXAMPLE);
	}

	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		isUsePlayer_[cursor_] =
			!isUsePlayer_[cursor_];
	}

	// 左
	if (ins.IsTrgDown(KEY_INPUT_LEFT))
	{
		cursor_--;

		if (cursor_ < 0)
		{
			cursor_ = 3;
		}
	}


	// 右
	if (ins.IsTrgDown(KEY_INPUT_RIGHT))
	{
		cursor_++;

		if (cursor_ > 3)
		{
			cursor_ = 0;
		}
	}

	// 使用プレイヤー更新
	/*for (int i = 0; i < 4; i++)
	{
		isUsePlayer_[i] = false;
	}*/

	// 選択人数分 true
	/*for (int i = 0; i < selectNum_ + 1; i++)
	{
		isUsePlayer_[i] = true;
	}*/
}

void PlayerNumScene::Draw(void)
{
	// 背景
	DrawGraph(
		0,
		0,
		bgImg_,
		FALSE
	);



	// プレイヤー数選択シーンの描画
	DrawString(100, 100, "プレイヤー数選択シーン", GetColor(255, 255, 255));


	for (int i = 0; i < SELECT_MAX; i++)
	{
		float scale = 0.7f;

		float y = 260;

		int playerColor = GetColor(255, 255, 255);

		switch (i)
		{
		case 0:
			playerColor = GetColor(255, 80, 80);
			break;

		case 1:
			playerColor = GetColor(80, 160, 255);
			break;

		case 2:
			playerColor = GetColor(80, 255, 120);
			break;

		case 3:
			playerColor = GetColor(255, 220, 80);
			break;
		}

		// カラーバー
		DrawBox(
			170 + i * 250,
			430,
			330 + i * 250,
			450,
			playerColor,
			TRUE
		);

		// 選択中の画像だけ拡大＋点滅
		if (i == cursor_)
		{

			
			scale = 1.0f;

			y += std::sin(GetNowCount() * 0.005f) * 10.0f;


			int alpha =
				(int)((std::sin(GetNowCount() * 0.01f)
					* 0.5f + 0.5f) * 255);

			// 枠は普通に描画
			/*SetDrawBlendMode(DX_BLENDMODE_NOBLEND,0);*/

			// 選択中の枠
			/*DrawBox(
				150 + i * 250,
				230,
				350 + i * 250,
				490,
				GetColor(255, 255, 255),
				FALSE
			);*/

			// 画像だけ点滅
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		}


		else
		{
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		DrawRotaGraph(
			250 + i * 250,
			(int)y,
			scale,
			0.0,
			selectImg_[i],
			TRUE
		);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// 画像下の USE / NOT USE1
		int color =
			isUsePlayer_[i]
			? GetColor(0, 255, 0)
			: GetColor(180, 180, 180);

		DrawGraph(
			170 + i * 250,
			520,
			isUsePlayer_[i] ? useImg_ : notUseImg_,
			TRUE
		);

		// カーソル矢印
		if (i == cursor_)
		{
			color = GetColor(255, 255, 255);

			DrawString(
				250 + i * 250,
				(int)y + 220,
				"▼",
				GetColor(255, 255, 255)
			);
		}
	}

	
}

void PlayerNumScene::Release(void)
{
	for (int i = 0; i < SELECT_MAX; i++)
	{
		DeleteGraph(selectImg_[i]);
	}

	DeleteGraph(useImg_);
	DeleteGraph(notUseImg_);
	DeleteGraph(bgImg_);


	DeleteSoundMem(decideSE_);
}
