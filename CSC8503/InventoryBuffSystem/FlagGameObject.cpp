#include "FlagGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "Vector3.h"
#include "map";
#include "PlayerInventory.h"
#include "PlayerObject.h"

using namespace NCL;
using namespace CSC8503;

FlagGameObject::FlagGameObject(InventoryBuffSystemClass* inventoryBuffSystemClassPtr, std::map<GameObject*, int>* playerObjectToPlayerNoMap, int pointsWorth)
	: Item(PlayerInventory::item::flag, *inventoryBuffSystemClassPtr) {
	mName = "Flag";
	mItemType = PlayerInventory::item::flag;
	mInventoryBuffSystemClassPtr = inventoryBuffSystemClassPtr;
	mPlayerObjectToPlayerNoMap = playerObjectToPlayerNoMap;
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(this);
	mPoints = pointsWorth;
}

FlagGameObject::~FlagGameObject() {
}

void FlagGameObject::GetFlag(int playerNo) {
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->AddItemToPlayer(InventoryBuffSystem::PlayerInventory::flag, playerNo);

	this->ToggleIsRendered();
}

void FlagGameObject::Reset() {
	if (!this->IsRendered())
		this->SetIsRendered(true);
}

void NCL::CSC8503::FlagGameObject::OnPlayerInteract(int playerId)
{
	if (this->IsRendered()) {
		GetFlag(playerId);
		SetIsRendered(false);
		SetHasPhysics(false);
	}
}


void FlagGameObject::UpdateInventoryObserver(InventoryEvent invEvent, int playerNo) {
	switch (invEvent) {
	case InventoryBuffSystem::flagDropped:
		Reset();
	default:
		break;
	}
}

//TODO - Eren/Kyriakos: we need a way of getting the player number for this player
void FlagGameObject::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->GetCollisionLayer() & Player && (this->IsRendered())
		&& this->mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->ItemInPlayerInventory(InventoryBuffSystem::PlayerInventory::flag, 0)) {
		GetFlag(0);
		SetIsRendered(false);
		SetHasPhysics(false);

		PlayerObject* plObj = (PlayerObject*)otherObject;
		plObj->AddPlayerPoints(mPoints);

	}
}
