#pragma once
#include <functional>
#include <map>
#include <random>
#include "Vector3.h"
#include "Level.h"

using namespace NCL::CSC8503;

using namespace NCL;
using namespace CSC8503;

namespace InventoryBuffSystem
{
	const enum InventoryEvent
	{
		flagDropped,disguiseItemUsed,soundEmitterUsed, screwdriverUsed
	};

	class PlayerInventoryObserver
	{
	public:
		virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved = false) = 0;
	};

	const int MAX_INVENTORY_SLOTS = 2;

	enum ItemUseType {
		DirectUse,
		NeedInteractableToUse
	};

	class PlayerInventory
	{
	public:

		enum item
		{
			none, disguise, soundEmitter, flag, screwdriver
		};

		PlayerInventory()
		{
			Init();
		}

		void Init();
		void AddItemToPlayer(item inItem, int playerNo);
		void DropItemFromPlayer(item inItem, int playerNo);
		void DropItemFromPlayer(int playerNo, int invSlot);
		void UseItemInPlayerSlot(int itemSlot, int playerNo, int itemUseCount);
		bool ItemInPlayerInventory(item inItem, int playerNo);
		bool HandleOnItemUsed(item item, int playerNo, int invSlot, int itemUseCount);

		void Attach(PlayerInventoryObserver* observer);
		void Detach(PlayerInventoryObserver* observer);
		void Notify(InventoryEvent invEvent, int playerNo, int invSLot, bool isItemRemoved = false);

		std::string& GetItemName(item item);

		PlayerInventory::item GetRandomItemFromPool(unsigned int seed);
		PlayerInventory::item GetPlayerItem(int playerId, int itemSlot);

		bool IsPlayerInventoryFull(int playerId);

	private:

		std::vector<item> mItemsInRandomPool = {
			screwdriver
		};

		std::map<item, InventoryEvent > mOnItemAddedInventoryEventMap = {

		};

		std::map<item, InventoryEvent > mOnItemDroppedInventoryEventMap = {
			{flag,flagDropped}
		};

		std::map<item, InventoryEvent > mOnItemUsedInventoryEventMap = {
			{disguise, disguiseItemUsed},
			{soundEmitter, soundEmitterUsed},
			{screwdriver, screwdriverUsed }
		};

		std::map<item, std::string> mItemNameMap = {
			{ screwdriver, "Screwdriver" },
			{ disguise, "Disguise" },
			{ soundEmitter, "Sound Emitter" },
			{ none, "No Equipped Item" }
		};

		std::map<item, int> mItemUsageToRemoveMap = {
			{ screwdriver, 2 },
			{ disguise, 1 },
			{ soundEmitter, 1 }
		};

		std::map<item, ItemUseType> mItemToItemUseTypeMap = {
			{ screwdriver, NeedInteractableToUse},
			{ disguise, DirectUse },
			{ soundEmitter, DirectUse },
			{ none, DirectUse }
		};

		std::map<item, std::function<bool(int playerNo)>> mItemPreconditionsMet;

		item mPlayerInventory[NCL::CSC8503::MAX_PLAYERS][MAX_INVENTORY_SLOTS];
		std::list<PlayerInventoryObserver*> mInventoryObserverList;
		void CreateItemPickup(item inItem, Vector3 Position) {}
	};


}
