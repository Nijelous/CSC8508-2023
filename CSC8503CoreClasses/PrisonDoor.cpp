#include "PrisonDoor.h"
#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"
#include "GameObject.h"

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

void PrisonDoor::Open() {
	GetTransform().SetPosition(GetTransform().GetPosition() + Vector3(0, 7.25, 0));
	mTimer = -1;
	mIsOpen = true;
}

void PrisonDoor::Close() {
	GetTransform().SetPosition(GetTransform().GetPosition() + Vector3(0, -7.25, 0));
	SetNavMeshFlags(2);
	mTimer = initDoorTimer;
	mIsOpen = false;
}

void PrisonDoor::UpdateObject(float dt) {
	if (!mIsOpen && mTimer > 0)
		CountDownTimer(dt);

	if (mTimer == 0)
		SetIsOpen(true);
}
