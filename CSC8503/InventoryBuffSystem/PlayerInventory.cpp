#include "PlayerInventory.h"
#include "Level.h"
#include "../CSC8503/LevelManager.h"

using namespace InventoryBuffSystem;
using namespace NCL::CSC8503;

void PlayerInventory::Init() {
	for (int playerNo = 0; playerNo < NCL::CSC8503::MAX_PLAYERS; playerNo++)
	{
		for (int invSlot = 0; invSlot < MAX_INVENTORY_SLOTS; invSlot++)
		{
			mPlayerInventory[playerNo][invSlot] = none;
		}
	}

	mItemPreconditionsMet =
	{
		{
			{none ,[](int playerno) { return false; }},
			{disguise,[](int playerno) { return true; }},
			{soundEmitter,[](int playerno) { return true; }},
			{flag ,[](int playerno) { return false; }},
			{screwdriver, [](int playerno) { return true; }},
			{doorKey ,[this](int playerno) { return PlayerAbleToUseItem[doorKey][playerno]; }},
		}
	};

	mInventoryObserverList.clear();
}

void PlayerInventory::AddItemToPlayer(item inItem, int playerNo) {
	for (int invSlot = 0; invSlot < MAX_INVENTORY_SLOTS; invSlot++)
	{
		if (mPlayerInventory[playerNo][invSlot] == none)
		{
			mPlayerInventory[playerNo][invSlot] = inItem;
			LevelManager::GetLevelManager()->ChangeEquippedIconTexture(invSlot, inItem);

			if (mOnItemAddedInventoryEventMap.find(inItem) != mOnItemAddedInventoryEventMap.end())
			{
				Notify(mOnItemAddedInventoryEventMap[inItem], playerNo, invSlot);
			}
			return;
		}
	}
}

void PlayerInventory::DropItemFromPlayer(item inItem, int playerNo) {
	for (int invSlot = 0; invSlot < MAX_INVENTORY_SLOTS; invSlot++)
	{
		if (mPlayerInventory[playerNo][invSlot] == inItem)
		{
			if (mOnItemDroppedInventoryEventMap.find(inItem) != mOnItemDroppedInventoryEventMap.end())
			{
				Notify(mOnItemDroppedInventoryEventMap[inItem], playerNo, invSlot);
			}

			mPlayerInventory[playerNo][invSlot] = none;
		}
	}
}

void PlayerInventory::DropItemFromPlayer(int playerNo, int invSlot) {
	if (mOnItemDroppedInventoryEventMap.find(mPlayerInventory[playerNo][invSlot]) != mOnItemDroppedInventoryEventMap.end())
	{
		Notify(mOnItemDroppedInventoryEventMap[mPlayerInventory[playerNo][invSlot]], playerNo, invSlot);
	}

	mPlayerInventory[playerNo][invSlot] = none;
}

void PlayerInventory::UseItemInPlayerSlot(int playerNo, int invSlot, int itemUseCount) {
	if (mOnItemUsedInventoryEventMap.find(mPlayerInventory[playerNo][invSlot]) != mOnItemUsedInventoryEventMap.end() &&
		mItemPreconditionsMet[mPlayerInventory[playerNo][invSlot]](playerNo))
	{
		PlayerInventory::item usedItem = mPlayerInventory[playerNo][invSlot];
		bool isItemRemoved = HandleOnItemUsed(usedItem, playerNo, invSlot, itemUseCount);
		Notify(mOnItemUsedInventoryEventMap[usedItem], (playerNo), invSlot, isItemRemoved);
		
	}
}

bool PlayerInventory::ItemInPlayerInventory(item inItem, int playerNo) {
	for (int invSlot = 0; invSlot < MAX_INVENTORY_SLOTS; invSlot++)
	{
		if (mPlayerInventory[playerNo][invSlot] == inItem)
		{
			return true;
		}
	}

	return false;
}

bool InventoryBuffSystem::PlayerInventory::HandleOnItemUsed(item item, int playerNo, int invSlot, int itemUseCount) {

	int maxUsage = mItemUsageToRemoveMap[item];

	if (itemUseCount >= maxUsage) {
		mPlayerInventory[playerNo][invSlot] = item::none;
		LevelManager::GetLevelManager()->ChangeEquippedIconTexture(invSlot, item::none);
		return true;
	}

	return false;
}

void InventoryBuffSystem::PlayerInventory::Attach(PlayerInventoryObserver* observer) {
	mInventoryObserverList.push_back(observer);
}

void InventoryBuffSystem::PlayerInventory::Detach(PlayerInventoryObserver* observer) {
	mInventoryObserverList.remove(observer);
}

void InventoryBuffSystem::PlayerInventory::Notify(const InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved) {
	std::list<PlayerInventoryObserver*>::iterator iterator = mInventoryObserverList.begin();
	while (iterator != mInventoryObserverList.end()) {
		(*iterator)->UpdateInventoryObserver(invEvent, playerNo, invSlot, isItemRemoved);
		++iterator;
	}
}

std::string& InventoryBuffSystem::PlayerInventory::GetItemName(item item) {
	return mItemNameMap[item];
}

PlayerInventory::item PlayerInventory::GetRandomItemFromPool(unsigned int seed) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(mItemsInRandomPool.begin(), mItemsInRandomPool.end(), gen);
	return mItemsInRandomPool[0];
}
