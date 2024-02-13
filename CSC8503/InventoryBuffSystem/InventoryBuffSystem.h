#pragma once
#include "PlayerBuffs.h"
#include "PlayerInventory.h"
//#include "PickupGameObject.h"

namespace InventoryBuffSystem
{
    class InventoryBuffSystemClass
    {
    public:

        InventoryBuffSystemClass()
        {
            mPlayerBuffsPtr = new PlayerBuffs();
            mPlayerInventoryPtr = new PlayerInventory();
        };

        void Reset()
        {
            mPlayerBuffsPtr->Init();
            mPlayerInventoryPtr->Init();
        };

        PlayerBuffs* GetPlayerBuffsPtr() { return mPlayerBuffsPtr; };
        PlayerInventory* GetPlayerInventoryPtr() { return mPlayerInventoryPtr; };
    private:
        PlayerBuffs* mPlayerBuffsPtr;
        PlayerInventory* mPlayerInventoryPtr;
    };
}

