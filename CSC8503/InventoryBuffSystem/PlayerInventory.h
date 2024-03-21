#pragma once
#include <functional>
#include <map>
#include "Vector3.h"
#include "LevelEnums.h"

using namespace NCL::CSC8503;

using namespace NCL;
using namespace CSC8503;

namespace InventoryBuffSystem
{
	const enum InventoryEvent
	{
		flagDropped,disguiseItemUsed,soundEmitterUsed, doorKeyUsed, screwdriverUsed, stunItemUsed, flagAdded
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

		const enum item
		{
			none, disguise, soundEmitter, flag, screwdriver, doorKey, stunItem
		};

		PlayerInventory()
		{
			Init();
		}

		void Init();
		int AddItemToPlayer(const item &inItem, const int &playerNo);
		int RemoveItemFromPlayer(const item& inItem, const int& playerNo);
		void RemoveItemFromPlayer(const int& playerNo, const int& invSlot);
		void DropItemFromPlayer(const item &inItem, const int &playerNo);
		void DropItemFromPlayer(const int &playerNo, const int &invSlot);
		void DropAllItemsFromPlayer(const int& playerNo);
		void UseItemInPlayerSlot(const int& playerNo, const int& invSlot);
		void OnItemEquipped(const int playerID, const int localPlayerID, const int slot, const item equippedItem);
		void ChangePlayerItem(const int playerID, const int localPlayerID, const int slotId, const item equippedItem, int usageCount);
		bool HandleItemRemoval(const item& item, const int& playerNo, const int& invSlot);
		bool ItemInPlayerInventory(const item &inItem, const int &playerNo);
		bool IsInventoryFull(const int& playerNo);
		bool IsInventoryEmpty(const int& playerNo);
		ItemUseType GetItemUseType(const item &inItem);

		void TransferItemBetweenInventories(const int& givingPlayerNo, const int& givingPlayerInvSlot, const int& receivingPlayerNo);

		void Attach(PlayerInventoryObserver* observer);
		void Detach(PlayerInventoryObserver* observer);
		void Notify(InventoryEvent invEvent, int playerNo, int invSLot, bool isItemRemoved = false);

		std::string& GetItemName(item item);
		int GetItemUsesLeft(const int& playerNo, const int& invSlot);

		PlayerInventory::item GetRandomItemFromPool(unsigned int seed, std::vector<item>* randomItemPool);
		PlayerInventory::item GetRandomItemFromPool(unsigned int seed, bool isSinglePlayer = true)
		{
			if(isSinglePlayer)
				return GetRandomItemFromPool(seed, &mItemsInSingleplayerRandomPool);
			else
				return GetRandomItemFromPool(seed, &mItemsInMultiplayerRandomPool);
		};

		PlayerInventory::item GetItemInInventorySlot(const int playerNo, const int itemSlot) { return mPlayerInventory[playerNo][itemSlot];  };
		
		int GetItemUsageCount( const int playerNo, const int itemSlot) { return mItemUseCount[playerNo][itemSlot]; };

		void SetPlayerAbleToUseItem(const item& inItem, const int& playerNo, const bool& isAbleToUseKey) {
			PlayerAbleToUseItem[inItem][playerNo] = isAbleToUseKey;
		};
	private:

		std::vector<item> mItemsInSingleplayerRandomPool = {
			doorKey, screwdriver, soundEmitter
		};

		std::vector<item> mItemsInMultiplayerRandomPool = {
			doorKey, screwdriver, soundEmitter
		};

		std::map<const item, const InventoryEvent > mOnItemAddedInventoryEventMap = {
			{flag,flagAdded}
		};

		std::map<const item, const InventoryEvent > mOnItemDroppedInventoryEventMap = {
			{flag,flagDropped}
		};

		std::map<const item, const InventoryEvent > mOnItemUsedInventoryEventMap = {
			{disguise, disguiseItemUsed},
			{soundEmitter, soundEmitterUsed},
			{screwdriver, screwdriverUsed },
			{doorKey,doorKeyUsed},
			{stunItem, stunItemUsed}
		};

		std::map<const item, std::string> mItemNameMap = {
			{ screwdriver,	"Screwdriver" },
			{ disguise,		"  Disguise " },
			{ soundEmitter, "  Boom Box " },
			{ doorKey,		" Door Key  " },
			{ stunItem,		" Stun Item " },
			{ flag,			" Heist Item" },
			{ none,			"  No Item  " }
		};

		std::map<const item, const int> mItemUsageToRemoveMap = {
			{ screwdriver, 2 },
			{ disguise, 1 },
			{ doorKey , 3 },
			{ soundEmitter, 1 },
			{ stunItem, 1}
		};

		std::map<const item, const ItemUseType> mItemToItemUseTypeMap = {
			{ doorKey , NeedInteractableToUse},
			{ screwdriver, NeedInteractableToUse},
			{ disguise, DirectUse },
			{ soundEmitter, DirectUse },
			{ stunItem, DirectUse},
			{ none, DirectUse }
		};

		std::map<item, std::function<bool(int playerNo)>> mItemPreconditionsMet;

		item mPlayerInventory[NCL::CSC8503::MAX_PLAYERS][MAX_INVENTORY_SLOTS];
		int mItemUseCount[NCL::CSC8503::MAX_PLAYERS][MAX_INVENTORY_SLOTS];
		std::list<PlayerInventoryObserver*> mInventoryObserverList;
		std::map < item ,bool[NCL::CSC8503::MAX_PLAYERS]> PlayerAbleToUseItem;
		void CreateItemPickup(item inItem, Maths::Vector3 Position) {}

		void IncreaseUsageCount(const int& playerNo, const int& invSlot) {
			mItemUseCount[playerNo][invSlot]++;
		}
		void ResetItemUsageCount(const int& playerNo, const int& invSlot) {
			mItemUseCount[playerNo][invSlot]=0;
		}
		void SetItemUsageCount(const int& playerNo, const int& invSlot, const int& usageCount) {
			mItemUseCount[playerNo][invSlot] = usageCount;
		}
	};


}
