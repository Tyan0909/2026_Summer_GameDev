#pragma once
#include "CameraBase.h"

class ZoomCamera : public CameraBase
{
public:
    void OnAcquire(Player* player) override;

    float GetScoreMultiplier() const override { return 1.2f; }
    bool CanZoom() const override { return true; }
    bool HasInsurance() const override { return false; }

    float GetNormalFOV() const override
    {
        return 60.0f;
    }

    float GetZoomFOV() const override
    {
        return 30.0f;
    }

};