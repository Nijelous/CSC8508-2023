#include "Door.h"
#include "InteractableDoor.h"
#include "NetworkObject.h"
#include "SoundObject.h"
#include "../CSC8503/DebugNetworkedGame.h"
#include "../CSC8503/SceneManager.h"
#include "PlayerObject.h"



using namespace NCL::CSC8503;

InteractableDoor::InteractableDoor() {
	GameObject::mName = "InteractableDoor";
	mInteractableItemType = InteractableItems::InteractableDoors;
	mIsLocked = false;
	mIsOpen = false;
	mTimer = -1;
}

void InteractableDoor::Unlock() {
	mIsLocked = false;
#ifdef USEGL
	this->GetSoundObject()->LockDoorTriggered();
#endif
	mLockCooldown = initLockCooldown;
	SetNavMeshFlags(2);
}

void InteractableDoor::Lock() {
	mIsLocked = true;
#ifdef USEGL
	this->GetSoundObject()->LockDoorTriggered();
#endif
	mLockCooldown = initLockCooldown;
	SetNavMeshFlags(4);
}

void InteractableDoor::Interact(InteractType interactType, GameObject* interactedObject)
{
	if (!CanBeInteractedWith(interactType, interactedObject))
		return;

	switch (interactType)
	{
	case Use:
		SetIsOpen(!mIsOpen);
		break;
	case LongUse:
		Unlock();
		break;
	case ItemUse:
		if (mIsLocked)
			Unlock();
		else
			Lock();
		break;
	default:
		break;
	}
}

bool InteractableDoor::CanBeInteractedWith(InteractType interactType, GameObject* interactedObject)
{
	switch (interactType)
	{
	case Use:
		return !mIsLocked;
		break;
	case ItemUse:
		return (mLockCooldown == 0 && CanUseItem(interactedObject) && !mIsOpen);
		break;
	case LongUse:
		return (mLockCooldown == 0 && mIsLocked && !mIsOpen);
		break;
	default:
		return false;
		break;
	}
}

bool InteractableDoor::CanUseItem(GameObject* userObj) {

	PlayerObject* playerComp = (PlayerObject*)userObj;
	if (playerComp == nullptr)
		return false;
	InventoryBuffSystem::PlayerInventory::item usedItem = playerComp->GetEquippedItem();

	switch (usedItem) {
	case InventoryBuffSystem::PlayerInventory::doorKey:
		return true;
		break;
	default:
		return false;
	}
}

#ifdef USEGL
void InteractableDoor::SyncInteractableDoorStatusInMultiplayer(bool toOpen) {
	auto* sceneManager = SceneManager::GetSceneManager();
	DebugNetworkedGame* networkedGame = static_cast<DebugNetworkedGame*>(sceneManager->GetCurrentScene());
	if (networkedGame) {
		auto* networkObj = GetNetworkObject();
		if (networkObj) {
			const int networkId = networkObj->GetnetworkID();
			if (networkedGame->GetServer()){
				networkedGame->SendInteractablePacket(networkId,toOpen,mInteractableItemType);
			}
		}
	}
}

void InteractableDoor::SyncDoor(bool toOpen){
	return;
}
#endif

void NCL::CSC8503::InteractableDoor::CountDownLockTimer(float dt)
{
	mLockCooldown = std::max(mLockCooldown - dt, 0.0f);
}

void InteractableDoor::UpdateGlobalSuspicionObserver(SuspicionSystem::SuspicionMetre::SusBreakpoint susBreakpoint) {
	switch (susBreakpoint)
	{
	case SuspicionSystem::SuspicionMetre::high:
		SetIsOpen(false);
		Lock();
		break;
	case SuspicionSystem::SuspicionMetre::mid:
		SetIsOpen(false);
		break;
	default:
		break;
	}
}

void InteractableDoor::UpdateObject(float dt) {
	if (mLockCooldown > 0)
		CountDownLockTimer(dt);

	if (!mIsLocked)
		Door::UpdateObject(dt);
}