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
    void Init();
    void Update();
    void Draw();
    void Release();

    void AddExplosion(const VECTOR& pos);
    void PlayExplosion(const VECTOR& pos);
    void PlaySmoke(const VECTOR& pos);
    void PlayHit(const VECTOR& pos);
    void PlayFlash(const VECTOR& pos);

private:

    std::vector<Explosion> explosions_;

    int effectBlastResId_ = -1;
    int effectSmokeResId_ = -1;
    int effectHitResId_ = -1;
    int effectFlashResId_ = -1;
};