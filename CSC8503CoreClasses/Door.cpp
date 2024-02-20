#include "Door.h"
#include "State.h"
#include "StateTransition.h"

using namespace NCL::CSC8503;

void Door::InitStateMachine()
{
	mStateMachine = new StateMachine();

	State* DoorOpen = new State([&](float dt) -> void
		{
			this->CountDownTimer(dt);

			if (mTimer == 0)
				Close();
		}
	);

	State* DoorClosed = new State([&](float dt) -> void
		{

		}
	);

	mStateMachine->AddState(DoorOpen);
	mStateMachine->AddState(DoorClosed);

	mStateMachine->AddTransition(new StateTransition(DoorOpen, DoorClosed,
		[&]() -> bool
		{
			return !mIsOpen;
		}
	));

	mStateMachine->AddTransition(new StateTransition(DoorClosed, DoorOpen,
		[&]() -> bool
		{
			return !mIsOpen;
		}
	));
}


void Door::Open()
{
	Vector3 UnderPos = GetTransform().GetPosition() + Vector3(0, -10, 0);
	GetTransform().SetPosition(UnderPos);
	mTimer = initDoorTimer;
	mIsOpen = true;
}

void Door::Close()
{
	Vector3 OverPos = GetTransform().GetPosition() + Vector3(0, 10, 0);
	GetTransform().SetPosition(OverPos);
	mIsOpen = false;
}

void Door::CountDownTimer(float dt)
{
	mTimer = std::max(mTimer - dt, 0.0f);
}
