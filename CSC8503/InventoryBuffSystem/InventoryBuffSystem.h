#pragma once
#include "PlayerBuffs.h"
#include "PlayerInventory.h"
//#include "PickupGameObject.h"

namespace InventoryBuffSystem
{
    InventoryBuffSystem::PlayerBuffs* mPlayerBuffsPtr;
    //InventoryBuffSystem::PlayerInventory* mPlayerInventoryPtr;

    void Init() 
    {
        mPlayerBuffsPtr = new PlayerBuffs();
        //mPlayerInventoryPtr = new PlayerInventory();
    };

    void Reset()
    {
        mPlayerBuffsPtr->Init();
        //mPlayerInventoryPtr->Init();
    };
}

