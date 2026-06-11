#include "EffectManager.h"


void EffectManager::Update()
{
    for (auto it = explosions_.begin();
        it != explosions_.end();)
    {
        it->frame++;

        if (it->frame > 30)
        {
            it = explosions_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void EffectManager::Draw()
{
    for (const auto& e : explosions_)
    {
        float radius =
            e.frame * 4.0f;

        SetDrawBlendMode(
            DX_BLENDMODE_ADD,
            180);

        DrawSphere3D(
            e.pos,
            radius,
            16,
            GetColor(255, 128, 0),
            GetColor(255, 128, 0),
            FALSE);

        DrawSphere3D(
            e.pos,
            radius * 0.5f,
            16,
            GetColor(255, 255, 0),
            GetColor(255, 255, 0),
            FALSE);

        SetDrawBlendMode(
            DX_BLENDMODE_NOBLEND,
            0);
    }
}

void EffectManager::AddExplosion(
    const VECTOR& pos)
{
    Explosion e;

    e.pos = pos;
    e.frame = 0;

    explosions_.push_back(e);
}