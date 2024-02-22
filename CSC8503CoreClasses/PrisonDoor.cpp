#include "PrisonDoor.h"
#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

using namespace NCL::CSC8503;

void PrisonDoor::UpdateGlobalSuspicionObserver(SuspicionSystem::SuspicionMetre::SusBreakpoint susBreakpoint){
	switch (susBreakpoint)
	{
	case SuspicionSystem::SuspicionMetre::high:
		Close();
		break;
	default:
		break;
	}
}

void PrisonDoor::InitStateMachine()
{
	mStateMachine = new StateMachine();

	State* DoorOpen = new State([&](float dt) -> void
		{

		}
	);

	State* DoorClosed = new State([&](float dt) -> void
		{
			this->CountDownTimer(dt);

			if (mTimer == 0)
				Open();
		}
	);

	mStateMachine->AddState(DoorClosed);
	mStateMachine->AddState(DoorOpen);

	mStateMachine->AddTransition(new StateTransition(DoorOpen, DoorClosed,
		[&]() -> bool
		{
			return (!this->mIsOpen);
		}
	));

	mStateMachine->AddTransition(new StateTransition(DoorClosed, DoorOpen,
		[&]() -> bool
		{
			return this->mIsOpen;
		}
	));
}

void PrisonDoor::UpdateObject(float dt){
	mStateMachine->Update(dt);
}
