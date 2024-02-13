#include "FlagGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "Vector3.h"
#include "map";

using namespace NCL;
using namespace CSC8503;

FlagGameObject::FlagGameObject(std::map<GameObject*, int>* playerObjectToPlayerNoMap, InventoryBuffSystemClass* inventoryBuffSystemClassPtr) {
	mPlayerObjectToPlayerNoMap = playerObjectToPlayerNoMap;
	mInventoryBuffSystemClassPtr = inventoryBuffSystemClassPtr;
}

FlagGameObject::~FlagGameObject() {
}

bool FlagGameObject::isServerPlayer(GameObject* otherObject)
{
	return (*mPlayerObjectToPlayerNoMap).find(otherObject) !=
		(*mPlayerObjectToPlayerNoMap).end();
}

void FlagGameObject::GetFlag(int playerNo)
{
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->AddItemToPlayer(InventoryBuffSystem::PlayerInventory::flag, playerNo);

	this->SetActive();
}

void FlagGameObject::Reset()
{
	if(!this->IsActive())
		this->SetActive();
}

void FlagGameObject::UpdateInventoryObserver(InventoryEvent invEvent, int playerNo)
{
	switch (invEvent)
	{
	case InventoryBuffSystem::flagDropped:
		Reset();
	default:
		break;
	}
}

void FlagGameObject::OnCollisionBegin(GameObject* otherObject)
{
	if (this->IsActive() && isServerPlayer(otherObject))
	{
		GetFlag((*mPlayerObjectToPlayerNoMap)[otherObject]);
	}
}
