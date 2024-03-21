#include "PointGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "Vector3.h"
#include "map";
#include "PlayerObject.h"
#include "SoundObject.h"
#include "State.h"

using namespace NCL;
using namespace CSC8503;

PointGameObject::PointGameObject(int pointsWorth, float initCooldown){
	mCooldown = 0;
	mPoints = pointsWorth;
	mInitCooldown = initCooldown;
	mName = "PickupGameObject";

	mStateMachine = new StateMachine();
	State* WaitingState = new State([&](float dt) -> void
		{
			this->Waiting(dt);
		}
	);
	State* InnactiveState = new State([&](float dt) -> void
		{
			this->Deactivate(dt);
		}
	);
	State* ActiveState = new State([&](float dt) -> void
		{
			this->Activate(dt);
		}
	);

	mStateMachine->AddState(WaitingState);
	mStateMachine->AddState(InnactiveState);
	mStateMachine->AddState(ActiveState);

	mStateMachine->AddTransition(new StateTransition(WaitingState, InnactiveState,
		[&]() -> bool
		{
			return this->mCooldown == INT_MAX;
		}
	));

	mStateMachine->AddTransition(new StateTransition(InnactiveState, WaitingState,
		[&]() -> bool
		{
			return this->mCooldown <= this->mInitCooldown;
		}
	));

	//InitCooldown -1 indicates a pickup that will not "Respawn"
	//So other transitions will not be necessary
	if (mInitCooldown == -1)
		return;

	mStateMachine->AddTransition(new StateTransition(WaitingState, ActiveState,
		[&]() -> bool
		{
			return this->mCooldown <= 0.2f && this->mCooldown > 0;
		}
	));

	mStateMachine->AddTransition(new StateTransition(ActiveState, WaitingState,
		[&]() -> bool
		{
			return this->mCooldown == 0;
		}
	));
}

PointGameObject::~PointGameObject() {
	delete mStateMachine;
}

void PointGameObject::UpdateObject(float dt) {
	mStateMachine->Update(dt);
}

void PointGameObject::Activate(float dt) {
	SetActive(true);
	mCooldown = 0;
}

void PointGameObject::Deactivate(float dt) {
	SetActive(false);
	mCooldown = mInitCooldown;
}

void PointGameObject::OnCollisionBegin(GameObject* otherObject) {
	if ((otherObject->GetCollisionLayer() & Player) &&
		mCooldown == 0) {
		PlayerObject* plObj = (PlayerObject*)otherObject;
		plObj->AddPlayerPoints(mPoints);
#ifdef USEGL
		this->GetSoundObject()->TriggerSoundEvent();
#endif
		mCooldown = INT_MAX;
	}
}

void PointGameObject::Waiting(float dt) {
	mCooldown = std::max(mCooldown - dt, 0.0f);
}
