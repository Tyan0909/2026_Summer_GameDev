#pragma once
#include "CameraBase.h"

class NormalCamera : public CameraBase
{
public:
    void OnAcquire(Player* player) override;

    float GetScoreMultiplier() const override { return 1.0f; }
    bool CanZoom() const override { return false; }
    bool HasInsurance() const override { return false; }

    float GetNormalFOV() const override
    {
        return 60.0f;
    }

    float GetZoomFOV() const override
    {
        return 60.0f;
    }
};