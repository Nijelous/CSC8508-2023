#pragma once
#include "GameObject.h"
#include "PlayerInventory.h"

namespace InventoryBuffSystem{
    class InventoryBuffSystemClass;

    class Item : public NCL::CSC8503::GameObject, public PlayerInventoryObserver {
    public:
        Item(PlayerInventory::item, InventoryBuffSystemClass& inventoryBuffSystemClass);
        
        PlayerInventory::item GetItemType() const { return mItemType;  };
        
        virtual void OnPlayerInteract(int playerId = 0) {}
    protected:
        PlayerInventory::item mItemType;
        InventoryBuffSystemClass* mInventoryBuffSystemClassPtr;
    };
}

