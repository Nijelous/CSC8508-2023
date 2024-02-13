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
            mPlayerInventoryPtr = new PlayerInventory();
            mPlayerBuffsPtr = new PlayerBuffs();
            mPlayerInventoryPtr->Attach(mPlayerBuffsPtr);
        };

        void Reset()
        {
            mPlayerBuffsPtr->Init();
            mPlayerInventoryPtr->Init();
        };

        void Update(float dt)
        {
            mPlayerBuffsPtr->Update(dt);
        };

        PlayerBuffs* GetPlayerBuffsPtr() { return mPlayerBuffsPtr; };
        PlayerInventory* GetPlayerInventoryPtr() { return mPlayerInventoryPtr; };
    private:
        PlayerBuffs* mPlayerBuffsPtr;
        PlayerInventory* mPlayerInventoryPtr;
    };
}

