#pragma once
#include <functional>
#include <map>
#include <random>
#include "Vector3.h"
#include "GameObject.h"
#include "../InventoryBuffSystem/InventoryBuffSystem.h"

using namespace NCL;
using namespace CSC8503;
using namespace InventoryBuffSystem;


namespace InventoryBuffSystem
{
	const int MAX_PLAYERS = 4;
	const int MAX_INVENTORY_SLOTS = 4;
	class PlayerInventory
	{
	public:
		enum item
		{
			disguise, item2,none,flag
		};
		PlayerInventory()
		{
			Init();
		}

		void Init();
		void AddItemToPlayer(item inItem, int playerNo); 
		void DropItemFromPlayer(item inItem, int playerNo);
		void DropItemFromPlayer(int playerNo, int invSlot);
		void DropFlagFromPlayer(int playerNo);
		void UseItemInPlayerSlot(int itemSlot, int playerNo);

		PlayerInventory::item GetRandomItemFromPool(unsigned int seed);
	private:

		std::vector<item> mItemsInRandomPool=
		{
			item2
		};

		std::map<item, std::function<void(int playerNo)>> mOnItemAddedFunctionMap =
		{
			{disguise, [](int playerNo)
				{

				}
			},
			{item2, [](int playerNo)
				{

				}
			},
		};

		std::map<item, std::function<void(int playerNo)>> mOnItemDroppedFunctionMap =
		{
			{disguise, [](int playerNo)
				{

				}
			},
			{item2, [](int playerNo)
				{

				}
			},
		};

		std::map<item, std::function<void(int playerNo)>> mOnItemUsedFunctionMap =
		{
			{disguise, [](int playerNo)
				{

				}
			},
			{item2, [](int playerNo)
				{

				}
			},
		};

		item mPlayerInventory[4][2];
		GameObject* mPlayerGameObjectsPTR;
		Vector3* mFlagLocationPTR;
		void CreateItemPickup(item inItem, Vector3 Position) {}
	};
}
