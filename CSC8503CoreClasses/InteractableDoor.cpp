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
	mName = "Door";

	bool isServer = SceneManager::GetSceneManager()->IsServer();
	if (isServer) {
		InitStateMachine();
	}
}

void InteractableDoor::Unlock() {
	SetIsOpen(false, false);
	SetNavMeshFlags(2);
}

void InteractableDoor::Lock() {
	SetIsOpen(true, false);
	SetNavMeshFlags(4);
}

void InteractableDoor::Interact(InteractType interactType, GameObject* interactedObject)
{
	if (!CanBeInteractedWith(interactType))
		return;

	switch (interactType)
	{
	case Use:
		SetIsOpen(!mIsOpen, true);
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

void InteractableDoor::SetIsOpen(bool isOpen, bool isSettedByServer) {
	mIsOpen = isOpen;
	if (isOpen) {
#ifdef USEGL
		this->GetSoundObject()->TriggerSoundEvent();
#endif
		SetActive(false);
		if (isSettedByServer) {
			mTimer = initDoorTimer;
		}
	}
	else {
#ifdef USEGL
		this->GetSoundObject()->CloseDoorTriggered();
#endif
		SetActive(true);
	}

#ifdef USEGL
	bool isMultiplayerGame = !SceneManager::GetSceneManager()->IsInSingleplayer();
	if (isMultiplayerGame && isSettedByServer) {
		SyncInteractableDoorStatusInMultiplayer();
	}
#endif
}

bool InteractableDoor::CanUseItem() {
#ifdef USEGL
	PlayerObject* localPlayer = LevelManager::GetLevelManager()->GetTempPlayer();
	PlayerInventory::item usedItem = localPlayer->GetEquippedItem();

	switch (usedItem) {
	case InventoryBuffSystem::PlayerInventory::doorKey:
		return true;
		break;
	default:
		return false;
	}
#endif
}

void InteractableDoor::InitStateMachine()
{
	mStateMachine = new StateMachine();

	State* DoorOpenAndUnlocked = new State([&](float dt) -> void
		{
			this->CountDownTimer(dt);

			if (mTimer == 0)
				SetIsOpen(false, true);
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
void InteractableDoor::SyncInteractableDoorStatusInMultiplayer() {
	auto* sceneManager = SceneManager::GetSceneManager();
	DebugNetworkedGame* networkedGame = static_cast<DebugNetworkedGame*>(sceneManager->GetCurrentScene());
	if (networkedGame) {
		auto* networkObj = GetNetworkObject();
		if (networkObj) {
			const int networkId = networkObj->GetnetworkID();

			SyncInteractablePacket packet(networkId, mIsOpen, mInteractableItemType);
			networkedGame->GetServer()->SendGlobalPacket(packet);
		}
	}
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