#include "Item.h"
#include "InventoryBuffSystem.h"

void InventoryBuffSystem::Item::AddToPlayerInventory(int playerId) {
	auto* playerInventory = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr();

	playerInventory->AddItemToPlayer(mItemType, playerId);
}
