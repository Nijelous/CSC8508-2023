#include "FlagGameObject.h"
#include "PlayerInventory.h"
#include "PlayerObject.h"
#include "../CSC8503/SoundObject.h"
using namespace NCL;
using namespace CSC8503;

FlagGameObject::FlagGameObject(InventoryBuffSystemClass* inventoryBuffSystemClassPtr, SuspicionSystemClass* suspicionSystemClassPtr,
	std::map<GameObject*, int>* playerObjectToPlayerNoMap, int pointsWorth)
	: Item(PlayerInventory::item::flag, *inventoryBuffSystemClassPtr) {
	mName = "Flag";
	mItemType = PlayerInventory::item::flag;
	mInventoryBuffSystemClassPtr = inventoryBuffSystemClassPtr;
	mSuspicionSystemClassPtr = suspicionSystemClassPtr;
	mPlayerObjectToPlayerNoMap = playerObjectToPlayerNoMap;
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(this);
	mPoints = pointsWorth;
}

FlagGameObject::~FlagGameObject() {
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Detach(this);
}

void FlagGameObject::GetFlag(int playerNo) {
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->AddItemToPlayer(InventoryBuffSystem::PlayerInventory::flag, playerNo);
	GetSoundObject()->TriggerSoundEvent();

	this->SetActive(false);
}

void FlagGameObject::Reset() {
	if (!this->IsActive())
	{
		this->SetActive(true);
	}
}

void NCL::CSC8503::FlagGameObject::OnPlayerInteract(int playerId)
{
	if (this->IsRendered()) {
		GetFlag(playerId);
		this->SetActive(false);
	}
}


void FlagGameObject::UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved) {
	switch (invEvent) {
	case InventoryBuffSystem::flagDropped:
		Reset();
	default:
		break;
	}
}

void FlagGameObject::UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo = 0) {
	switch (buffEvent) {
#ifdef USEGL
	case BuffEvent::flagSightApplied:
		SetIsSensed(true);
		break;
	case BuffEvent::flagSightRemoved:
		SetIsSensed(false);
#endif
	default:
		break;
	}
}

void FlagGameObject::OnCollisionBegin(GameObject* otherObject) {
	if ((otherObject->GetCollisionLayer() & Player)) {
		PlayerObject* plObj = (PlayerObject*)otherObject;
		const int playerID = plObj->GetPlayerID();
		if (!mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->IsInventoryFull(playerID)){
			plObj->AddPlayerPoints(mPoints);
			GetFlag(playerID);
		}
	}
}
