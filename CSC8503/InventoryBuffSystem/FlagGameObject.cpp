#include "FlagGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "Vector3.h"
#include "map";
#include "PlayerInventory.h"

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

void FlagGameObject::OnCollisionBegin(GameObject* otherObject) {
	if (this->IsRendered() || this->HasPhysics()) {
		GetFlag(0);
		SetIsRendered(false);
		SetHasPhysics(false);
	}
}
