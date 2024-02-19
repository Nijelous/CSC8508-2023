#include "Door.h"
#include "InteractableDoor.h"
#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

using namespace NCL::CSC8503;

void InteractableDoor::Unlock(){
	mIsLocked = false;
}

void InteractableDoor::Lock(){
	mIsLocked = true;
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
		return !mIsOpen;
	case LongUse:
		return (mIsLocked && !mIsOpen);
		break;
	default:
		return false;
		break;
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

	State* DoorOpenAndLocked = new State([&](float dt) -> void
		{

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
	mStateMachine->AddState(DoorOpenAndLocked);
	mStateMachine->AddState(DoorOpenAndUnlocked);
	mStateMachine->AddState(DoorClosedAndLocked);

	mStateMachine->AddTransition(new StateTransition(DoorOpenAndUnlocked, DoorClosedAndUnlocked,
		[&]() -> bool
		{
			return (!this->mIsOpen);
		}
	));

	mStateMachine->AddTransition(new StateTransition(DoorOpenAndUnlocked, DoorOpenAndLocked,
		[&]() -> bool
		{
			return this->mIsLocked;
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