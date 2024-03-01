#include "Door.h"
#include "InteractableDoor.h"
#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"
#include "../CSC8503/LevelManager.h"

using namespace NCL::CSC8503;

void InteractableDoor::Unlock(){
	mIsLocked = false;
	SetNavMeshFlags(2);
}

void InteractableDoor::Lock(){
	mIsLocked = true;
	SetNavMeshFlags(4);
}

void InteractableDoor::Interact(InteractType interactType)
{
	if (!CanBeInteractedWith(interactType))
		return;

	switch (interactType)
	{
	case Use:
		if (mIsOpen)
			Close();
		else
			Open();
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

bool InteractableDoor::CanUseItem(){
	auto* localPlayer = LevelManager::GetLevelManager()->GetTempPlayer();
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
				Close();
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

void InteractableDoor::UpdateGlobalSuspicionObserver(SuspicionSystem::SuspicionMetre::SusBreakpoint susBreakpoint){
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