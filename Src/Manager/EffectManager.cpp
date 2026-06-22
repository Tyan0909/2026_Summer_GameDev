#include "EffectManager.h"
#include <DxLib.h>
#include <fstream>
#include <EffekseerForDXLib.h>
#include "../Application.h"
#include "../Scene/GameScene.h"

void EffectManager::Init()
{

    int h =
        LoadEffekseerEffect(
            "Data/Effect/Blast/Blast.efkefc");

    std::string path =
        Application::PATH_EFFECT +
        "Blast/Blast.efkefc";

    effectBlastResId_ =
        LoadEffekseerEffect(
            path.c_str());

    // 煙
    effectSmokeResId_ =
        LoadEffekseerEffect(
            (Application::PATH_EFFECT +
                "Smoke.efkefc").c_str());

    // ヒット
    effectHitResId_ =
        LoadEffekseerEffect(
            (Application::PATH_EFFECT +
                "Hit.efkefc").c_str());

    // カメラフラッシュ
    effectFlashResId_ =
        LoadEffekseerEffect(
            (Application::PATH_EFFECT +
                "Flash.efkefc").c_str());

    printfDx(
        "BlastResId=%d\n",
        effectBlastResId_);

    int effectHandle =
        PlayEffekseer3DEffect(
            effectBlastResId_);

    printfDx(
        "Handle=%d\n",
        effectHandle);

}

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
    UpdateEffekseer3D();
}

void EffectManager::Draw()
{

    DrawFormatString(
        0,
        200,
        GetColor(255, 255, 255),
        "EffectManager Draw");

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
    DrawEffekseer3D();

    DrawSphere3D(
        VGet(300, 100, 1000),
        50,
        16,
        GetColor(0, 255, 0),
        GetColor(0, 255, 0),
        TRUE);
}

void EffectManager::Release()
{
    DeleteEffekseerEffect(effectBlastResId_);
    DeleteEffekseerEffect(effectSmokeResId_);
    DeleteEffekseerEffect(effectHitResId_);
    DeleteEffekseerEffect(effectFlashResId_);

    effectBlastResId_ = -1;
    effectSmokeResId_ = -1;
    effectHitResId_ = -1;
    effectFlashResId_ = -1;
}

// ここではDxLibのエフェクトを再生する
void EffectManager::AddExplosion(
    const VECTOR& pos)
{

    printfDx(
        "AddExplosion %.1f %.1f %.1f\n",
        pos.x,
        pos.y,
        pos.z);

    Explosion e;

    e.pos = pos;
    e.frame = 0;

    explosions_.push_back(e);


}

// ここではEffekseerのエフェクトも再生する
void EffectManager::PlayExplosion(
    const VECTOR& pos)
{

    int effectHandle =
        PlayEffekseer3DEffect(effectBlastResId_);


    if (effectHandle != -1)
    {
        SetPosPlayingEffekseer3DEffect(
            effectHandle,
            pos.x,
            pos.y,
            pos.z);

        SetScalePlayingEffekseer3DEffect(
            effectHandle,
            100.0f,
            100.0f,
            100.0f);
    }

    if (effectBlastResId_ == -1)
    {
        DrawFormatString(
            0, 100,
            GetColor(255, 0, 0),
            "Load Failed");
        return;
    }

    if (effectHandle != -1)
    {
        SetPosPlayingEffekseer3DEffect(
            effectHandle,
            pos.x,
            pos.y,
            pos.z);
    }
}
void EffectManager::PlaySmoke(
    const VECTOR& pos)
{
    if (effectSmokeResId_ == -1)
    {
        return;
    }

    int effectHandle =
        PlayEffekseer3DEffect(
            effectSmokeResId_);

    if (effectHandle != -1)
    {
        SetPosPlayingEffekseer3DEffect(
            effectHandle,
            pos.x,
            pos.y,
            pos.z);
    }
}

void EffectManager::PlayHit(
    const VECTOR& pos)
{
    if (effectHitResId_ == -1)
    {
        return;
    }

    int effectHandle =
        PlayEffekseer3DEffect(
            effectBlastResId_);

    if (effectHandle != -1)
    {
        SetPosPlayingEffekseer3DEffect(
            effectHandle,
            pos.x,
            pos.y,
            pos.z);
    }
}

void EffectManager::PlayFlash(
    const VECTOR& pos)
{
    if (effectFlashResId_ == -1)
    {
        return;
    }

    int effectHandle =
        PlayEffekseer3DEffect(
            effectBlastResId_);

    if (effectHandle != -1)
    {
        SetPosPlayingEffekseer3DEffect(
            effectHandle,
            pos.x,
            pos.y,
            pos.z);
    }
}

