#include "ItemFactory.h"
#include <memory>
#include "Camera/CameraBase.h"
#include "Camera/NormalCamera.h"
#include "Camera/ZoomCamera.h"
#include "Camera/InsuranceCamera.h"

std::unique_ptr<CameraBase> ItemFactory::Create(ITEM_TYPE type)
{
    switch (type)
    {
    case ITEM_TYPE::NORMAL_CAMERA:
        return std::make_unique<NormalCamera>();

    case ITEM_TYPE::ZOOM_CAMERA:
        return std::make_unique<ZoomCamera>();

    case ITEM_TYPE::INSURANCE_CAMERA:
        return std::make_unique<InsuranceCamera>();

    default:
        return nullptr;
    }
}
