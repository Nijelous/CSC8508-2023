#pragma once
#include "PlayerBuffs.h"
#include "PlayerInventory.h"
//#include "PickupGameObject.h"

namespace InventoryBuffSystem
{
    PlayerBuffs* mPlayerBuffsPtr;
    PlayerInventory* mPlayerInventoryPtr;

    void Init() 
    {
        mPlayerBuffsPtr = new PlayerBuffs();
        mPlayerInventoryPtr = new PlayerInventory();
    };

    void Reset()
    {
        mPlayerBuffsPtr->Init();
        mPlayerInventoryPtr->Init();
    };
}

