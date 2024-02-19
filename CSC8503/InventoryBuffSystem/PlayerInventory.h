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
		virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo) = 0;
	};

	constexpr int MAX_INVENTORY_SLOTS = 2;

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
		void UseItemInPlayerSlot(int itemSlot, int playerNo);
		bool ItemInPlayerInventory(item inItem, int playerNo);

		void Attach(PlayerInventoryObserver* observer);
		void Detach(PlayerInventoryObserver* observer);
		void Notify(InventoryEvent invEvent,int playerNo);

		PlayerInventory::item GetRandomItemFromPool(unsigned int seed);
		PlayerInventory::item GetItemInInventorySlot(int itemSlot, int playerNo) { return mPlayerInventory[playerNo][itemSlot];  };
	
		void SetPlayerAbleToUseItem(item inItem, int playerNo, bool isAbleToUseKey) {
			PlayerAbleToUseItem[inItem][playerNo] = isAbleToUseKey;
		};
	private:

		std::vector<item> mItemsInRandomPool =
		{
			screwdriver, disguise
		};

		std::map<item, InventoryEvent > mOnItemAddedInventoryEventMap =
		{

		};

		std::map<item, InventoryEvent > mOnItemDroppedInventoryEventMap =
		{
			{flag,flagDropped}
		};

		std::map<item, InventoryEvent > mOnItemUsedInventoryEventMap =
		{
			{disguise, disguiseItemUsed}, {soundEmitter, soundEmitterUsed}, {doorKey,doorKeyUsed}
		};
		
		std::map<item, std::function<bool(int playerNo)>> mItemPreconditionsMet;

		item mPlayerInventory[NCL::CSC8503::MAX_PLAYERS][MAX_INVENTORY_SLOTS];
		std::list<PlayerInventoryObserver*> mInventoryObserverList;
		std::map < item ,bool[NCL::CSC8503::MAX_PLAYERS]> PlayerAbleToUseItem;
		void CreateItemPickup(item inItem, Vector3 Position) {}
	};


}
