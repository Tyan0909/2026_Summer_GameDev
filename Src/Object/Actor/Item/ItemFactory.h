#pragma once
#include <memory>
#include "../../../Scene/BuySelect.h"

class ItemBase;
class CameraBase;

class ItemFactory
{
public:

    static std::unique_ptr<CameraBase> Create(ITEM_TYPE type);

};