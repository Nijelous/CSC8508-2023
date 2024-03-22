#include "PlayerInventory.h"

#include "GameServer.h"
#include "GameClient.h"
#include "NetworkObject.h"
#include "../LevelManager.h"
#include "../DebugNetworkedGame.h"
#include "../LevelManager.h"
#include "../SceneManager.h"
#include <algorithm>
#include <random>

namespace {
	constexpr int DEFAULT_ITEM_USAGE_COUNT = 0;
	
}



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

			int localPlayerId = 0;

#ifdef USEGL
			if (!SceneManager::GetSceneManager()->IsInSingleplayer()) {
				DebugNetworkedGame* game = reinterpret_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
				auto* localPlayer = game->GetLocalPlayer();
				localPlayerId = localPlayer->GetPlayerID();

				//if it's server, inform clients
				if (game->GetIsServer()) {
					game->SendClientSyncItemSlotPacket(playerNo, invSlot, inItem, DEFAULT_ITEM_USAGE_COUNT);
				}
				else
				{
					game->GetClient()->WriteAndSendInventoryPacket(playerNo, invSlot, item::none, DEFAULT_ITEM_USAGE_COUNT);
				}
			}
#endif

			OnItemEquipped(playerNo, localPlayerId, invSlot, inItem);

			if (mOnItemAddedInventoryEventMap.find(inItem) != mOnItemAddedInventoryEventMap.end()) {
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

void PlayerInventory::RemoveItemFromPlayer(const int& playerNo, const int& invSlot){
	ResetItemUsageCount(playerNo, invSlot);
	LevelManager::GetLevelManager()->DropEquippedIconTexture(invSlot);
	mPlayerInventory[playerNo][invSlot] = none;

	//Potentially move the multiplayer related code below to a function like HandleMultiplayerItemRemoval(...);
	int localPlayerId = 0;
#ifdef USEGL
	DebugNetworkedGame* game = reinterpret_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
	if (!SceneManager::GetSceneManager()->IsInSingleplayer()) {
		const auto* localPlayer = game->GetLocalPlayer();
		localPlayerId = localPlayer->GetPlayerID();

		const bool isServer = game->GetIsServer();
		if (isServer) {
			game->SendClientSyncItemSlotPacket(playerNo, invSlot, item::none, DEFAULT_ITEM_USAGE_COUNT);
		}
		else
		{
			game->GetClient()->WriteAndSendInventoryPacket(playerNo, invSlot, item::none, DEFAULT_ITEM_USAGE_COUNT);
		}
	}

	if (localPlayerId == playerNo) {
		LevelManager::GetLevelManager()->ChangeEquippedIconTexture(invSlot, item::none);
	}
#endif
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
	if (mOnItemDroppedInventoryEventMap.find(mPlayerInventory[playerNo][invSlot]) !=
		mOnItemDroppedInventoryEventMap.end())
	{
		Notify(mOnItemDroppedInventoryEventMap[mPlayerInventory[playerNo][invSlot]], playerNo, invSlot);
	}
	RemoveItemFromPlayer(playerNo, invSlot);
	//Extra drop logic
}

void PlayerInventory::DropAllItemsFromPlayer(const int& playerNo){
	for (int i = 0; i < MAX_INVENTORY_SLOTS; i++)
		DropItemFromPlayer(playerNo,i);
}

void PlayerInventory::UseItemInPlayerSlot(const int& playerNo, const int& invSlot) {
	if (mOnItemUsedInventoryEventMap.find(mPlayerInventory[playerNo][invSlot]) != mOnItemUsedInventoryEventMap.end() &&
		mItemPreconditionsMet[mPlayerInventory[playerNo][invSlot]](playerNo)) {
		PlayerInventory::item usedItem = mPlayerInventory[playerNo][invSlot];
		IncreaseUsageCount(playerNo, invSlot);
		bool isItemRemoved = HandleItemRemoval(usedItem, playerNo, invSlot);
		std::cout << "Player(" + std::to_string(playerNo) + ") used: " + GetItemName(usedItem) << '\n';

		int localPlayerId = 0;

#ifdef USEGL
		if (!SceneManager::GetSceneManager()->IsInSingleplayer()) {
			DebugNetworkedGame* game = reinterpret_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
			auto* localPlayer = game->GetLocalPlayer();
			localPlayerId = localPlayer->GetPlayerID();

			//if it's server, inform clients
			if (game->GetIsServer()) {
				int usageCount = mItemUseCount[playerNo][invSlot];
				game->SendClientSyncItemSlotPacket(playerNo, invSlot, usedItem, usageCount);
			}
			else
			{
				int item = mPlayerInventory[playerNo][invSlot];
				int usageCount = mItemUseCount[playerNo][invSlot];
				game->GetClient()->WriteAndSendInventoryPacket(playerNo, invSlot, item, usageCount);
			}
		}
#endif

		Notify(mOnItemUsedInventoryEventMap[usedItem], (playerNo), invSlot, isItemRemoved);
	}
}

void PlayerInventory::OnItemEquipped(const int playerID, const int localPlayerID, const int slot, const item equippedItem) {
	if (localPlayerID == playerID) {
		LevelManager::GetLevelManager()->ChangeEquippedIconTexture(slot, equippedItem);
	}
}


void PlayerInventory::ChangePlayerItem(const int playerID, const int localPlayerID, const int slotId, const item equippedItem, int usageCount) {
	mPlayerInventory[playerID][slotId] = equippedItem;
	mItemUseCount[playerID][slotId] = usageCount;
	OnItemEquipped(playerID, localPlayerID, slotId, equippedItem);
}

bool InventoryBuffSystem::PlayerInventory::HandleItemRemoval(const item& item, const int& playerNo, const int& invSlot) {

	int maxUsage = mItemUsageToRemoveMap[item];
	const int currentItemUseCount = mItemUseCount[playerNo][invSlot];

	if (currentItemUseCount >= maxUsage) {
		RemoveItemFromPlayer(playerNo, invSlot);
		return true;
	}

	return false;
}

bool PlayerInventory::ItemInPlayerInventory(const item& inItem, const int& playerNo) {
	if (playerNo < 0 || playerNo > 3) {
		return false;
	}

	for (int invSlot = 0; invSlot < MAX_INVENTORY_SLOTS; invSlot++)
	{
		if (mPlayerInventory[playerNo][invSlot] == inItem)
		{
			return true;
		}
	}

	return false;
}

bool PlayerInventory::IsInventoryFull(const int& playerNo) {
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
	item itemToRemove = GetItemInInventorySlot(givingPlayerNo, givingPlayerInvSlot);
	if (IsInventoryFull(receivingPlayerNo) || itemToRemove == none)
		return;
	int receivingSlot = AddItemToPlayer(itemToRemove, receivingPlayerNo);
	SetItemUsageCount(receivingPlayerNo, receivingSlot, GetItemUsageCount(givingPlayerNo, givingPlayerInvSlot));
	RemoveItemFromPlayer(givingPlayerNo, givingPlayerInvSlot);
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
