#pragma once
#include "../ItemBase.h"

class CameraBase : public ItemBase
{
public:
    virtual ~CameraBase() = default;

    virtual void OnAcquire(Player* player) override
    {
        // デフォルトでは何もしない
    }

    virtual float GetScoreMultiplier() const
    {
        return 1.0f;
    }

    virtual float GetNormalFOV() const
    {
        return 60.0f;
    }

    virtual float GetZoomFOV() const
    {
        return 60.0f;
    }

    virtual bool CanZoom() const
    {
        return false;
    }

    virtual bool HasInsurance() const
    {
        return false;
    }
};