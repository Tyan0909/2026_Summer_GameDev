#include "Result.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/PhotoManager.h"
#include <DxLib.h>
#include "../Manager/SoundManager.h"
#include <algorithm>

Result::Result(void) { printf("Result Constructor\n"); }
Result::~Result(void) {}

void Result::Init(void)
{
    resultFrame_ = 0;
    displayScore_ = 0;
    scoreDisplay_ = 0;
    scoreFinished_ = false;
    shineFrame_ = 0;

    // 画面の上からスタート
    photoY_ = -500;

    SceneManager& scene =
        SceneManager::GetInstance();

    isClear_ =
        scene.GetGameResult() ==
        SceneManager::GAME_RESULT::CLEAR;


	SoundManager::GetInstance().StopBgm();

	SoundManager::GetInstance().PlaySe(
		ResourceManager::SRC::GAMECLEAR_SE);

	const PhotoData* best =
		PhotoManager::GetInstance().GetBestPhoto();

	if (best)
	{
		bestPhotoHandle_ = best->graphHandle;
		bestPhotoScore_ = best->score;
		bestPhotoPlayer_ = best->playerIndex;
	}

	titleFont_ = CreateFontToHandle(
		"Impact",
		60,
		3);

    rankFont_ = CreateFontToHandle(
        "Meiryo",
        28,     // サイズ
        5);     // 太さ（1～9くらい）

    std::vector<int> scores =
        SceneManager::GetInstance().GetPlayerScore();

    ranking_.clear();

    for (int i = 0; i < scores.size(); i++)
    {
        ranking_.push_back({ i + 1, scores[i] });
    }

    std::sort(
        ranking_.begin(),
        ranking_.end(),
        [](const RankData& a, const RankData& b)
        {
            return a.score > b.score;
        });
	
}

void Result::Update(void)
{
	InputManager& ins = InputManager::GetInstance();
	SceneManager& scene = SceneManager::GetInstance();

	const auto padNo = InputManager::JOYPAD_NO::PAD1;

	// Xボタン
	if (
		ins.IsTrgDown(KEY_INPUT_SPACE) ||
		ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::LEFT)
		)
	{
		scene.SetCarryMoney(0);
		scene.SetGameResult(SceneManager::GAME_RESULT::NONE);
		scene.SetPhotoCount(0);
		scene.SetLastPhotoScore(0);
		scene.ChangeScene(SceneManager::SCENE_ID::TITLE);
		return;
	}

	// Aボタン
	if (
		ins.IsTrgDown(KEY_INPUT_RETURN) ||
		ins.IsPadBtnTrgDown(padNo, InputManager::JOYPAD_BTN::DOWN)
		)
	{
		if (scene.GetGameResult() == SceneManager::GAME_RESULT::CLEAR)
		{
			scene.ChangeScene(SceneManager::SCENE_ID::BUYSELECT);
		}
		else
		{
			scene.SetCarryMoney(0);
			scene.SetGameResult(SceneManager::GAME_RESULT::NONE);
			scene.SetPhotoCount(0);
			scene.SetLastPhotoScore(0);
			scene.ChangeScene(SceneManager::SCENE_ID::TITLE);
		}
	}

    
	resultFrame_++;

    if (photoY_ < 110)
    {
        photoY_ += (110 - photoY_) / 5 + 1;
    }
    else if (photoY_ > 100)
    {
        photoY_--;
    }

    const int targetScore =
        SceneManager::GetInstance().GetCarryMoney();

    if (!scoreFinished_)
    {
        if (scoreDisplay_ < targetScore)
        {
            int add = max(1, (targetScore - scoreDisplay_) / 12);
            scoreDisplay_ += add;

            if (scoreDisplay_ >= targetScore)
            {
                scoreDisplay_ = targetScore;

                // カウント終了
                scoreFinished_ = true;

                // キラ演出開始
                shineFrame_ = 30;
            }
        }
    }

    if (shineFrame_ > 0)
    {
        shineFrame_--;
    }
}

void Result::Draw(void)
{
    SceneManager& scene = SceneManager::GetInstance();

    //==================================================
    // 背景
    //==================================================
    DrawBox(
    0,
    0,
    1280,
    720,
    GetColor(8, 12, 25),
    TRUE);

    //==================================================
    // フィルム風ライン
    //==================================================
    for (int y = 0; y < 720; y += 4)
    {
        DrawLine(
            0,
            y,
            1280,
            y,
            GetColor(15, 20, 35));
    }

    //==================================================
    // 背景グリッド
    //==================================================
    for (int x = 0; x < 1280; x += 80)
    {
        DrawLine(
            x,
            0,
            x,
            720,
            GetColor(18, 35, 55));
    }

    for (int y = 0; y < 720; y += 80)
    {
        DrawLine(
            0,
            y,
            1280,
            y,
            GetColor(18, 35, 55));
    }

    const int totalScore = scene.GetCarryMoney();
    const int photoCount = scene.GetPhotoCount();
    const int lastPhotoScore = scene.GetLastPhotoScore();

    const char* title =
        isClear_ ? "GAME CLEAR" : "GAME OVER";

    const int titleColor =
        isClear_?
        GetColor(0, 255, 120) :
        GetColor(255, 80, 80);

    //-------------------------------------------------
    // GAME CLEAR
    //-------------------------------------------------

    // 影
    DrawStringToHandle(
        304,
        54,
        title,
        GetColor(30, 30, 30),
        titleFont_);

    // 本体
    DrawStringToHandle(
        300,
        50,
        title,
        titleColor,
        titleFont_);

    //-------------------------------------------------
    // 文字を順番に表示

    if (resultFrame_ > 150)
    {
        for (int i = 0; i < ranking_.size(); i++)
        {
            if (resultFrame_ < 150 + i * 20)
                continue;

            int color = GetColor(255, 255, 255);

            if (i == 0)
                color = GetColor(255, 215, 0);      // 金
            else if (i == 1)
                color = GetColor(220, 220, 220);    // 銀
            else if (i == 2)
                color = GetColor(205, 127, 50);     // 銅

            char str[64];

            sprintf_s(
                str,
                "%d. PLAYER%d   %d",
                i + 1,
                ranking_[i].playerNo,
                ranking_[i].score);

            // 影
            DrawStringToHandle(
                72,
                332 + i * 40,
                str,
                GetColor(20, 20, 20),
                rankFont_);

            // 本体
            DrawStringToHandle(
                70,
                330 + i * 40,
                str,
                color,
                rankFont_);
        }
    }

    if (shineFrame_ > 0)
    {
        int alpha = shineFrame_ * 8;
        if (alpha > 255) alpha = 255;

        SetDrawBlendMode(
            DX_BLENDMODE_ADD,
            alpha);

        DrawLine(
            520,
            225,
            560,
            265,
            GetColor(255, 255, 255));

        DrawLine(
            560,
            225,
            520,
            265,
            GetColor(255, 255, 255));

        DrawCircle(
            540,
            245,
            6,
            GetColor(255, 255, 200),
            TRUE);

        SetDrawBlendMode(
            DX_BLENDMODE_NOBLEND,
            0);
    }

    if (resultFrame_ > 60 && isClear_)
    {
        DrawFormatString(
            60,
            210,
            GetColor(255, 255, 255),
            "PHOTO COUNT : %d",
            photoCount);
    }

    if (resultFrame_ > 90 && isClear_)
    {
        DrawFormatString(
            60,
            250,
            GetColor(0, 255, 255),
            "LAST PHOTO : +%d",
            lastPhotoScore);
    }



    //-------------------------------------------------
    // ベストショット
    //-------------------------------------------------

    if (bestPhotoHandle_ != -1)
    {
        const int photoX = 420;
        const int photoY = photoY_;
        const int photoW = 500;
        const int photoH = 360;


        //==================================================
        // スポットライト
        //==================================================
        SetDrawBlendMode(
            DX_BLENDMODE_ALPHA,
            35);

        DrawCircle(
            photoX + photoW / 2,
            photoY + photoH / 2,
            300,
            GetColor(255, 255, 255),
            TRUE);

        SetDrawBlendMode(
            DX_BLENDMODE_NOBLEND,
            0);

        // 影
        DrawBox(
            photoX + 8,
            photoY + 8,
            photoX + photoW + 8,
            photoY + photoH + 8,
            GetColor(40, 40, 40),
            TRUE);

        // 白いポラロイド
        DrawBox(
            photoX,
            photoY,
            photoX + photoW,
            photoY + photoH,
            GetColor(255, 255, 255),
            TRUE);

        // 外枠
        DrawBox(
            photoX,
            photoY,
            photoX + photoW,
            photoY + photoH,
            GetColor(220, 220, 220),
            FALSE);

        // 写真
        DrawExtendGraph(
            photoX + 18,
            photoY + 18,
            photoX + photoW - 18,
            photoY + photoH - 70,
            bestPhotoHandle_,
            TRUE);

        // BEST SHOT
        DrawString(
            photoX + 20,
            photoY + photoH - 42,
            "BEST SHOT",
            GetColor(0, 0, 0));

        // PLAYER
        DrawFormatString(
            photoX + 250,
            photoY + photoH - 42,
            GetColor(0, 0, 0),
            "PLAYER %d",
            bestPhotoPlayer_ + 1);

        // SCORE
        DrawFormatString(
            photoX + 20,
            photoY + photoH - 20,
            GetColor(0, 0, 0),
            "SCORE %d",
            bestPhotoScore_);

        // TOTAL SCORE
        DrawStringToHandle(
            photoX + 140,
            photoY + photoH + 20,
            "TOTAL SCORE",
            GetColor(255, 255, 255),
            rankFont_);

        // 数字を少し大きなフォントで表示するとさらに見栄えUP
        DrawFormatString(
            photoX + 185,
            photoY + photoH + 55,
            GetColor(255, 220, 0),
            "%d",
            scoreDisplay_);

    }

    //-------------------------------------------------
    // 操作説明
    //-------------------------------------------------

    if (resultFrame_ > 120)
    {
        if (isClear_)
        {
            DrawString(
                60,
                620,
                "ENTER : BUY SELECT",
                GetColor(255, 255, 255));
        }
        else
        {
            DrawString(
                60,
                620,
                "ENTER : TITLE",
                GetColor(255, 255, 255));
        }

        DrawString(
            60,
            650,
            "SPACE : TITLE",
            GetColor(255, 255, 255));
    }
}
void Result::Release()
{
    if (rankFont_ != -1)
    {
        DeleteFontToHandle(rankFont_);
        rankFont_ = -1;
    }

	if (titleFont_ != -1)
	{
		DeleteFontToHandle(titleFont_);
		titleFont_ = -1;
	}
}