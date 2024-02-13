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
		flagDropped,disguiseItemUsed,soundEmitterUsed
	};

	class PlayerInventoryObserver
	{
	public:
		virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo) = 0;
	};

	const int MAX_INVENTORY_SLOTS = 2;

	class PlayerInventory
	{
	public:

		enum item
		{
			disguise, soundEmitter, flag,  none
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

	private:

		std::vector<item> mItemsInRandomPool =
		{
			disguise, soundEmitter
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
			{disguise, disguiseItemUsed}, {soundEmitter, soundEmitterUsed}
		};

		item mPlayerInventory[NCL::CSC8503::MAX_PLAYERS][MAX_INVENTORY_SLOTS];
		PlayerBuffs* mPlayerBuffsPtr;
		std::list<PlayerInventoryObserver*> mInventoryObserverList;
		void CreateItemPickup(item inItem, Vector3 Position) {}
	};


}
