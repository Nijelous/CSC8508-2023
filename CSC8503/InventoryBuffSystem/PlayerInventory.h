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
		flagDropped,disguiseItemUsed,soundEmitterUsed, doorKeyUsed, screwdriverUsed
	};

	class PlayerInventoryObserver
	{
	public:
		virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved = false) = 0;
	};

	constexpr int MAX_INVENTORY_SLOTS = 2;

	enum ItemUseType {
		DirectUse,
		NeedInteractableToUse
	};

	class PlayerInventory
	{
	public:

		enum item
		{
			none, disguise, soundEmitter, flag, screwdriver, doorKey
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
		ItemUseType GetItemUseType(item inItem);


		void Attach(PlayerInventoryObserver* observer);
		void Detach(PlayerInventoryObserver* observer);
		void Notify(InventoryEvent invEvent, int playerNo, int invSLot, bool isItemRemoved = false);

		std::string& GetItemName(item item);

		PlayerInventory::item GetRandomItemFromPool(unsigned int seed);
		PlayerInventory::item GetItemInInventorySlot(int itemSlot, int playerNo) { return mPlayerInventory[playerNo][itemSlot];  };
	
		void SetPlayerAbleToUseItem(item inItem, int playerNo, bool isAbleToUseKey) {
			PlayerAbleToUseItem[inItem][playerNo] = isAbleToUseKey;
		};
	private:

		std::vector<item> mItemsInRandomPool = {
			doorKey
		};

		std::map<item, InventoryEvent > mOnItemAddedInventoryEventMap = {

		};

		std::map<item, InventoryEvent > mOnItemDroppedInventoryEventMap = {
			{flag,flagDropped}
		};

		std::map<item, InventoryEvent > mOnItemUsedInventoryEventMap = {
			{disguise, disguiseItemUsed},
			{soundEmitter, soundEmitterUsed},
			{screwdriver, screwdriverUsed },
			{doorKey,doorKeyUsed}
		};

		std::map<item, std::string> mItemNameMap = {
			{ screwdriver, "Screwdriver" },
			{ disguise, "Disguise" },
			{ soundEmitter, "Sound Emitter" },
			{ doorKey, "Door Key" },
			{ none, "No Equipped Item" }
		};

		std::map<item, int> mItemUsageToRemoveMap = {
			{ screwdriver, 2 },
			{ disguise, 1 },
			{ doorKey ,1 },
			{ soundEmitter, 1 }
		};

		std::map<item, ItemUseType> mItemToItemUseTypeMap = {
			{ doorKey , NeedInteractableToUse},
			{ screwdriver, NeedInteractableToUse},
			{ disguise, DirectUse },
			{ soundEmitter, DirectUse },
			{ none, DirectUse }
		};

		std::map<item, std::function<bool(int playerNo)>> mItemPreconditionsMet;

		item mPlayerInventory[NCL::CSC8503::MAX_PLAYERS][MAX_INVENTORY_SLOTS];
		std::list<PlayerInventoryObserver*> mInventoryObserverList;
		std::map < item ,bool[NCL::CSC8503::MAX_PLAYERS]> PlayerAbleToUseItem;
		void CreateItemPickup(item inItem, Vector3 Position) {}
	};


}
