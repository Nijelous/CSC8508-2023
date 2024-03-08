#include "Door.h"
#include "InteractableDoor.h"

#include "GameServer.h"
#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"
#include "NetworkObject.h"
#include "../CSC8503/DebugNetworkedGame.h"
#include "../CSC8503/SceneManager.h"

using namespace NCL::CSC8503;

InteractableDoor::InteractableDoor() {
	GameObject::mName = "InteractableDoor";
	mInteractableItemType = InteractableItems::InteractableDoors;
	mIsLocked = false;
	mIsOpen = false;

	bool isServer = SceneManager::GetSceneManager()->IsServer();
	if (true) {
		InitStateMachine();
	}
}

void InteractableDoor::Unlock() {
	SetIsOpen(false);
	SetNavMeshFlags(2);
}

void InteractableDoor::Lock() {
	SetIsOpen(true);
	SetNavMeshFlags(4);
}

void InteractableDoor::Interact(InteractType interactType, GameObject* interactedObject)
{
	if (!CanBeInteractedWith(interactType))
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

bool InteractableDoor::CanBeInteractedWith(InteractType interactType)
{
	switch (interactType)
	{
	case Use:
		return !mIsLocked;
		break;
	case ItemUse:
		return CanUseItem() && !mIsOpen;
		break;
	case LongUse:
		return (mIsLocked && !mIsOpen);
		break;
	default:
		return false;
		break;
	}
}

void InteractableDoor::SetIsOpen(bool toOpen) {

	auto* sceneManager = SceneManager::GetSceneManager();
	DebugNetworkedGame* networkedGame = static_cast<DebugNetworkedGame*>(sceneManager->GetCurrentScene());

	if (toOpen) {
		if (networkedGame->GetIsServer())
		{
			Open();
			mTimer = initDoorTimer;
		}
		else
		{
			SetIsRendered(false);
			mTimer = initDoorTimer;
		}
	}
	else {
		//this->GetSoundObject()->CloseDoorTriggered();
		if (networkedGame->GetIsServer())
		{
			Close();
		}
		else
			SetIsRendered(true);
	}

	mIsOpen = toOpen;
	bool isMultiplayerGame = !SceneManager::GetSceneManager()->IsInSingleplayer();
	if (isMultiplayerGame) {
		SyncInteractableDoorStatusInMultiplayer(toOpen);
	}
}

bool InteractableDoor::CanUseItem() {
	PlayerObject* localPlayer = LevelManager::GetLevelManager()->GetTempPlayer();
	PlayerInventory::item usedItem = localPlayer->GetEquippedItem();

	switch (usedItem) {
	case InventoryBuffSystem::PlayerInventory::doorKey:
		return true;
		break;
	default:
		return false;
	}
}

void InteractableDoor::InitStateMachine()
{
	mStateMachine = new StateMachine();

	State* DoorOpenAndUnlocked = new State([&](float dt) -> void
		{
			this->CountDownTimer(dt);

			if (mTimer == 0)
				SetIsOpen(false);
		}
	);

	State* DoorClosedAndUnlocked = new State([&](float dt) -> void
		{

		}
	);


	State* DoorClosedAndLocked = new State([&](float dt) -> void
		{

		}
	);

	mStateMachine->AddState(DoorClosedAndUnlocked);
	mStateMachine->AddState(DoorOpenAndUnlocked);
	mStateMachine->AddState(DoorClosedAndLocked);

	mStateMachine->AddTransition(new StateTransition(DoorOpenAndUnlocked, DoorClosedAndUnlocked,
		[&]() -> bool
		{
			return (!this->mIsOpen);
		}
	));

	mStateMachine->AddTransition(new StateTransition(DoorClosedAndUnlocked, DoorOpenAndUnlocked,
		[&]() -> bool
		{
			return this->mIsOpen;
		}
	));

	mStateMachine->AddTransition(new StateTransition(DoorClosedAndUnlocked, DoorClosedAndLocked,
		[&]() -> bool
		{
			return this->mIsLocked;
		}
	));

	mStateMachine->AddTransition(new StateTransition(DoorClosedAndLocked, DoorClosedAndUnlocked,
		[&]() -> bool
		{
			return !this->mIsLocked;
		}
	));
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
				SyncInteractablePacket packet(networkId, toOpen, mInteractableItemType);
				networkedGame->GetServer()->SendGlobalPacket(packet);
			}
		}
	}
}

void InteractableDoor::SyncDoor(bool toOpen){
	SetIsRendered(!toOpen);
}
#endif

void InteractableDoor::UpdateGlobalSuspicionObserver(SuspicionSystem::SuspicionMetre::SusBreakpoint susBreakpoint) {
	switch (susBreakpoint)
	{
	case SuspicionSystem::SuspicionMetre::high:
		Close();
		break;
	default:
		break;
	}
}



void InteractableDoor::UpdateObject(float dt) {
	mStateMachine->Update(dt);
}