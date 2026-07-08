#pragma once
#include "CameraBase.h"

class InsuranceCamera : public CameraBase
{
public:
    void OnAcquire(Player* player) override;

    float GetScoreMultiplier() const override { return 0.8f; }
    bool CanZoom() const override { return false; }
    bool HasInsurance() const override { return true; }

    float GetNormalFOV() const override
    {
        return 60.0f;
    }

    float GetZoomFOV() const override
    {
        return 60.0f;
    }

};