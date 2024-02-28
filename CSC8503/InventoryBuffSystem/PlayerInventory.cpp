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
			{flag ,[](int playerno) { return false; }},
			{disguise,[](int playerno) { return true; }},
			{soundEmitter,[](int playerno) { return true; }},
			{screwdriver, [](int playerno) { return true; }},
			{doorKey ,[this](int playerno) { return true; }},
			{ stunItem, [this](int playerno) { return true; }}
		}
	};

	mInventoryObserverList.clear();
}

//Returns the inventory slot the item is in or -1 if inventory is full
int PlayerInventory::AddItemToPlayer(const item& inItem, const int& playerNo) {
	if (IsInventoryFull(playerNo))
		return -1;

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
			return invSlot;
		}
	}
	return -1;
}

//Returns the inventory slot the item is in or -1 if it was not found
int PlayerInventory::RemoveItemFromPlayer(const item& inItem, const int& playerNo){
	for (int invSlot = 0; invSlot < MAX_INVENTORY_SLOTS; invSlot++)
	{
		if (mPlayerInventory[playerNo][invSlot] == inItem)
		{
			RemoveItemFromPlayer(playerNo, invSlot);
			return invSlot;
		}
	}
	return -1;
}

void InventoryBuffSystem::PlayerInventory::RemoveItemFromPlayer(const int& playerNo, const int& invSlot){
	ResetItemUsageCount(playerNo, invSlot);
	LevelManager::GetLevelManager()->DropEquippedIconTexture(invSlot);
	mPlayerInventory[playerNo][invSlot] = none;
}

void PlayerInventory::DropItemFromPlayer(const item& inItem, const int& playerNo) {
	int invSlot = RemoveItemFromPlayer(inItem, playerNo);
	if (invSlot!=-1 &&
		mOnItemDroppedInventoryEventMap.find(inItem) != mOnItemDroppedInventoryEventMap.end())
	{
		Notify(mOnItemDroppedInventoryEventMap[inItem], playerNo, invSlot);
	}
	//Extra drop logic
}

void PlayerInventory::DropItemFromPlayer(const int& playerNo, const int& invSlot) {
	RemoveItemFromPlayer(playerNo, invSlot);
	if (mOnItemDroppedInventoryEventMap.find(mPlayerInventory[playerNo][invSlot]) != mOnItemDroppedInventoryEventMap.end())
	{
		Notify(mOnItemDroppedInventoryEventMap[mPlayerInventory[playerNo][invSlot]], playerNo, invSlot);
	}
	//Extra drop logic
}

void PlayerInventory::UseItemInPlayerSlot(const int& playerNo, const int& invSlot) {
	if (mOnItemUsedInventoryEventMap.find(mPlayerInventory[playerNo][invSlot]) != mOnItemUsedInventoryEventMap.end() &&
		mItemPreconditionsMet[mPlayerInventory[playerNo][invSlot]](playerNo))
	{
		PlayerInventory::item usedItem = mPlayerInventory[playerNo][invSlot];
		IncreaseUsageCount(playerNo, invSlot);
		bool isItemRemoved = HandleItemRemoval(usedItem, playerNo, invSlot);
		Notify(mOnItemUsedInventoryEventMap[usedItem], (playerNo), invSlot, isItemRemoved);
	}
}

bool InventoryBuffSystem::PlayerInventory::HandleItemRemoval(const item& item, const int& playerNo, const int& invSlot) {

	int maxUsage = mItemUsageToRemoveMap[item];

	if (mItemUseCount[playerNo][invSlot] >= maxUsage) {
		RemoveItemFromPlayer(playerNo, invSlot);
		return true;
	}

	return false;
}

bool PlayerInventory::ItemInPlayerInventory(const item& inItem, const int& playerNo) {
	for (int invSlot = 0; invSlot < MAX_INVENTORY_SLOTS; invSlot++)
	{
		if (mPlayerInventory[playerNo][invSlot] == inItem)
		{
			return true;
		}
	}

	return false;
}

bool PlayerInventory::IsInventoryFull(const int& playerNo){
	for (int invSlot = 0; invSlot < MAX_INVENTORY_SLOTS; invSlot++)
		if (mPlayerInventory[playerNo][invSlot] == none)
			return false;
	return true;
}

bool InventoryBuffSystem::PlayerInventory::IsInventoryEmpty(const int& playerNo){
	for (int invSlot = 0; invSlot < MAX_INVENTORY_SLOTS; invSlot++)
		if (mPlayerInventory[playerNo][invSlot] != none)
			return false;
	return true;
}

ItemUseType InventoryBuffSystem::PlayerInventory::GetItemUseType(const item& inItem) {
	return mItemToItemUseTypeMap[inItem];
}

void InventoryBuffSystem::PlayerInventory::TransferItemBetweenInventories(const int& givingPlayerNo, const int& givingPlayerInvSlot, const int& receivingPlayerNo){
	if (IsInventoryFull(receivingPlayerNo) || IsInventoryEmpty(givingPlayerNo))
		return;
	item itemToRemove = GetItemInInventorySlot(givingPlayerNo, givingPlayerInvSlot);
	RemoveItemFromPlayer(givingPlayerNo, givingPlayerInvSlot);
	int receivingSlot = AddItemToPlayer(itemToRemove, receivingPlayerNo);
	SetItemUsageCount(receivingPlayerNo, receivingSlot, GetItemUsageCount(givingPlayerNo, givingPlayerInvSlot));
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

int PlayerInventory::GetItemUsesLeft(const int& playerNo, const int& itemSlot)
{
	return mItemUsageToRemoveMap[GetItemInInventorySlot(playerNo, itemSlot)]
		- mItemUseCount[playerNo][itemSlot];
}

PlayerInventory::item PlayerInventory::GetRandomItemFromPool(unsigned int seed, std::vector<item>* randomItemPool) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle((*randomItemPool).begin(), (*randomItemPool).end(), gen);
	return (*randomItemPool)[0];
}
