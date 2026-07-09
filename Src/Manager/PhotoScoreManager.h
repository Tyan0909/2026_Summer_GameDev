#pragma once

#include <vector>
#include "../Object/Actor/Item/Trap/Trap.h"

class Player;
class Subject;
class GameScene;

class PhotoScoreManager
{
private:

    static constexpr int PHOTO_SCORE_MAX = 1000;
    static constexpr int PHOTO_SCORE_MIN = 300;

	// ŽB‰e‹——£‚É‚æ‚éƒXƒRƒAŒvŽZ‚Ìè‡’l
    static constexpr float PHOTO_SCORE_NEAR_DISTANCE = 80.0f;
    static constexpr float PHOTO_SCORE_FAR_DISTANCE = 450.0f;

public:

    int CalculateScore(
        const Player* player,
        const Subject* subject,
        const GameScene* scene) const;

private:

    int CalculateDistanceScore(
        const Player* player,
        const Subject* subject) const;

    float CalculateDirectionScore(
        const Player* player,
        const Subject* subject) const;

    int CalculateDangerScore(
        const Player* player,
        const Subject* subject,
        const GameScene* scene) const;
};