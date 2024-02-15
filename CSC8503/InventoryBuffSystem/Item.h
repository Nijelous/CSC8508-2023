#pragma once
#include "GameObject.h"
#include "PlayerInventory.h"

namespace InventoryBuffSystem{
    class InventoryBuffSystemClass;

    class Item : public NCL::CSC8503::GameObject, public PlayerInventoryObserver {
    public:
        virtual void AddToPlayerInventory(int playerId = 0);
        
    protected:
        PlayerInventory::item mItemType;
        InventoryBuffSystemClass* mInventoryBuffSystemClassPtr;
    };
}

