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
            mPlayerInventoryPtr->Attach(mPlayerBuffsPtr);
        };

        void Update(float dt)
        {
            mPlayerBuffsPtr->Update(dt);
        };

        ~InventoryBuffSystemClass() {
            mPlayerInventoryPtr->Detach(mPlayerBuffsPtr);
            delete mPlayerBuffsPtr;
            delete mPlayerInventoryPtr;
        }

        PlayerBuffs* GetPlayerBuffsPtr() { return mPlayerBuffsPtr; };
        PlayerInventory* GetPlayerInventoryPtr() { return mPlayerInventoryPtr; };
    private:
        PlayerBuffs* mPlayerBuffsPtr;
        PlayerInventory* mPlayerInventoryPtr;
    };
}

