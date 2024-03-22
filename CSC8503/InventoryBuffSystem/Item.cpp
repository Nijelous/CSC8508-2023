#include "Item.h"
#include "InventoryBuffSystem.h"

InventoryBuffSystem::Item::Item(PlayerInventory::item itemType, InventoryBuffSystemClass& inventoryBuffSystemClass) {
	mItemType = itemType;
	mInventoryBuffSystemClassPtr = &inventoryBuffSystemClass;
}

//void InventoryBuffSystem::Item::OnPlayerInteract(int playerId) {
//}


