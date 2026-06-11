#pragma once
#include <vector>
#include <DxLib.h>

class EffectManager
{
public:

    struct Explosion
    {
        VECTOR pos;
        int frame;
    };

    void Update();
    void Draw();

    void AddExplosion(
        const VECTOR& pos);

private:

    std::vector<Explosion> explosions_;
};