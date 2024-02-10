	#include "PlayerInventory.h"
	#include "Level.h"

	using namespace InventoryBuffSystem;
	using namespace NCL::CSC8503;

	void PlayerInventory::Init()
	{
		for (int playerNo = 0; playerNo < NCL::CSC8503::MAX_PLAYERS; playerNo++)
		{
			for (int invSlot = 0; invSlot < MAX_INVENTORY_SLOTS; invSlot++)
			{
				mPlayerInventory[playerNo][invSlot] = none;
			}
		}

	}

	void PlayerInventory::AddItemToPlayer(item inItem, int playerNo)
	{
		for (int invSlot = 0; invSlot < MAX_INVENTORY_SLOTS; invSlot++)
		{
			if (mPlayerInventory[playerNo][invSlot] == none)
			{
				mPlayerInventory[playerNo][invSlot] = inItem;

				if (mOnItemAddedFunctionMap.find(inItem) != mOnItemAddedFunctionMap.end())
				{
					mOnItemAddedFunctionMap[inItem](playerNo);
				}
				return;
			}
		}
	}

	void PlayerInventory::DropItemFromPlayer(item inItem, int playerNo)
	{
		for (int invSlot = 0; invSlot < MAX_INVENTORY_SLOTS; invSlot++)
		{
			if (mPlayerInventory[playerNo][invSlot] == inItem)
			{
				CreateItemPickup(inItem, (mPlayerGameObjectsPTR + playerNo)->GetTransform().GetPosition());

				if (mOnItemDroppedFunctionMap.find(inItem) != mOnItemDroppedFunctionMap.end())
				{
					mOnItemDroppedFunctionMap[inItem](playerNo);
				}

				mPlayerInventory[playerNo][invSlot] = none;
			}
		}
	}

	void PlayerInventory::DropItemFromPlayer(int playerNo, int invSlot)
	{
		CreateItemPickup(mPlayerInventory[playerNo][invSlot],
			(mPlayerGameObjectsPTR + playerNo)->GetTransform().GetPosition());

		if (mOnItemDroppedFunctionMap.find(mPlayerInventory[playerNo][invSlot]) != mOnItemDroppedFunctionMap.end())
		{
			mOnItemDroppedFunctionMap[mPlayerInventory[playerNo][invSlot]](playerNo);
		}

		mPlayerInventory[playerNo][invSlot] = none;
	}


	void PlayerInventory::DropFlagFromPlayer(int playerNo)
	{
		for (int invSlot = 0; invSlot < MAX_INVENTORY_SLOTS; invSlot++)
		{
			if (mPlayerInventory[playerNo][invSlot] == flag)
			{
				CreateItemPickup(flag, *mFlagLocationPTR);
				mPlayerInventory[playerNo][invSlot] = none;
			}
		}
	}

	void PlayerInventory::UseItemInPlayerSlot(int playerNo, int invSlot)
	{
		if (mOnItemUsedFunctionMap.find(mPlayerInventory[playerNo][invSlot]) != mOnItemUsedFunctionMap.end())
		{
			mOnItemUsedFunctionMap[mPlayerInventory[playerNo][invSlot]](playerNo);
		}

		mPlayerInventory[playerNo][invSlot] = none;
	}

	PlayerInventory::item PlayerInventory::GetRandomItemFromPool(unsigned int seed)
	{
		std::mt19937 rng(seed);
		std::shuffle(mItemsInRandomPool.begin(), mItemsInRandomPool.end(), rng);
		return mItemsInRandomPool[0];
	}
