#include "Door.h"
#include "State.h"
#include "StateTransition.h"
#include "../CSC8503/LevelManager.h"

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
	SetActive(false);
	mTimer = initDoorTimer;
	mIsOpen = true;
	SetNavMeshFlags(1);
	
}

void Door::Close()
{
	SetActive(true);
	mIsOpen = false;
	SetNavMeshFlags(2);
}

void Door::CountDownTimer(float dt)
{
	mTimer = std::max(mTimer - dt, 0.0f);
}

void Door::SetNavMeshFlags(int flag) {
	float* pos = new float[3] { mTransform.GetPosition().x, mTransform.GetPosition().y, mTransform.GetPosition().z };
	AABBVolume* volume = (AABBVolume*)mBoundingVolume;
	float* halfSize = new float[3] { volume->GetHalfDimensions().x, volume->GetHalfDimensions().y, volume->GetHalfDimensions().z };
	LevelManager::GetLevelManager()->LoadDoorInNavGrid(pos, halfSize, (PolyFlags)flag);
}
