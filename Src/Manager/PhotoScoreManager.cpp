#include "PhotoScoreManager.h"

#include "../Scene/GameScene.h"
#include "../Object/Actor/Charactor/Player/Player.h"
#include "../Object/Actor/Charactor/Subject/Subject.h"
#include "../Object/Actor/Item/Trap/Trap.h"

int PhotoScoreManager::CalculateScore(const Player* player, const Subject* subject, const GameScene* scene) const
{
	// 撮影距離スコア
    int distanceScore =
        CalculateDistanceScore(player, subject);

	// 撮影方向スコア
    float directionRate =
        CalculateDirectionScore(player, subject);

	// 危険度スコア
    int dangerBonus =
        CalculateDangerScore(player, subject, scene);
	// 撮影距離スコアに方向スコアを掛けて、危険度ボーナスを加算
    int score =
        static_cast<int>(distanceScore * directionRate);

    score += dangerBonus;

    return score;
}

int PhotoScoreManager::CalculateDistanceScore(const Player* player,const Subject* subject) const
{
    const VECTOR shotPos =
        player->GetCameraWorldPos();

    const VECTOR targetPos =
        subject->GetTransform().pos;

    const float distance =
        VSize(VSub(targetPos, shotPos));

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

    int score =
        static_cast<int>(
            PHOTO_SCORE_MAX -
            (PHOTO_SCORE_MAX - PHOTO_SCORE_MIN) * t);

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

float PhotoScoreManager::CalculateDirectionScore(const Player* player,const Subject* subject) const
{
    // 被写体の前方向
    VECTOR forward = subject->GetForward();

    // 被写体→カメラ
    VECTOR toCamera =
        VSub(
            player->GetCameraWorldPos(),
            subject->GetTransform().pos);

    toCamera = VNorm(toCamera);

    float dot = VDot(forward, toCamera);

    if (dot >= 0.9f)
    {
        return 300;
    }

    if (dot >= 0.6f)
    {
        return 200;
    }

    if (dot >= 0.3f)
    {
        return 100;
    }

    return 0;
}

int PhotoScoreManager::CalculateDangerScore(
    const Player* player,
    const Subject* subject,
    const GameScene* scene) const
{
    if (player == nullptr ||
        subject == nullptr ||
        scene == nullptr)
    {
        return 0;
    }


    int bonus = 0;

    const VECTOR subjectPos =
        subject->GetTransform().pos;


    /*const auto& traps =
        scene->GetTraps();*/


    constexpr float DANGER_DISTANCE = 300.0f;
    constexpr int MAX_DANGER_BONUS = 300;


    float nearDistance = FLT_MAX;
    const Trap* dangerTrap = nullptr;


    // 一番近い罠を探す
    /*for (const auto& trap : traps)
    {
        VECTOR diff =
            VSub(
                trap.pos,
                subjectPos);

        diff.y = 0.0f;

        float dist = VSize(diff);


        if (dist < nearDistance)
        {
            nearDistance = dist;
            dangerTrap = &trap;
        }
    }*/


    if (dangerTrap == nullptr)
    {
        return 0;
    }


    //--------------------------------------------------
    // ① 罠との距離ボーナス
    //--------------------------------------------------
    if (nearDistance <= DANGER_DISTANCE)
    {
        float rate =
            1.0f -
            (nearDistance / DANGER_DISTANCE);


        bonus += static_cast<int>(
            MAX_DANGER_BONUS * rate);


        if (dangerTrap->type == TRAP_TYPE::MINE)
        {
            bonus += 50;
        }
    }


    //--------------------------------------------------
    // ② 爆発直前ボーナス
    //--------------------------------------------------
    if (dangerTrap->type == TRAP_TYPE::MINE &&
        dangerTrap->triggered)
    {
        if (dangerTrap->lifeFrames <= 10)
        {
            bonus += 300;
        }
        else if (dangerTrap->lifeFrames <= 20)
        {
            bonus += 150;
        }
    }


    //--------------------------------------------------
    // ③ スタン中ボーナス
    //--------------------------------------------------
    if (subject->IsStunned())
    {
        bonus += 100;
    }


    //--------------------------------------------------
    // ④ 瀕死・爆発状態ボーナス
    //--------------------------------------------------
    if (subject->IsDying())
    {
        bonus += 200;
    }


    return bonus;
}