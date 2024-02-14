#include "PickupGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "Vector3.h"
#include "random"
#include "InventoryBuffSystem.h"

using namespace NCL;
using namespace CSC8503;
using namespace InventoryBuffSystem;

PickupGameObject::PickupGameObject(
	InventoryBuffSystemClass* inventoryBuffSystemClassPtr,
	std::map<GameObject*, int>* playerObjectToPlayerNoMap,
	float initCooldown) {
	mCooldown = 0.0f;
	mInitCooldown = initCooldown;
	mInventoryBuffSystemClassPtr = inventoryBuffSystemClassPtr;
	mPlayerObjectToPlayerNoMap = playerObjectToPlayerNoMap;

	mStateMachine = new StateMachine();
	State* WaitingState = new State([&](float dt) -> void
		{
			this->Waiting(dt);
		}
	);
	State* GoUnderState = new State([&](float dt) -> void
		{
			this->GoUnder(dt);
		}
	);
	State* GoOverState = new State([&](float dt) -> void
		{
			this->GoOver(dt);
		}
	);

	mStateMachine->AddState(WaitingState);
	mStateMachine->AddState(GoUnderState);
	mStateMachine->AddState(GoOverState);

	mStateMachine->AddTransition(new StateTransition(WaitingState, GoUnderState,
		[&]() -> bool
		{
			return this->mCooldown == INT_MAX;
		}
	));

	mStateMachine->AddTransition(new StateTransition(GoUnderState, WaitingState,
		[&]() -> bool
		{
			return this->mCooldown <= this->mInitCooldown;
		}
	));

	//InitCooldown -1 indicates a pickup that will not "Respawn"
	//So other transitions will not be necessary
	if (mInitCooldown == -1)
		return;

	mStateMachine->AddTransition(new StateTransition(WaitingState, GoOverState,
		[&]() -> bool
		{
			return this->mCooldown <= 0.2f && this->mCooldown > 0;
		}
	));

	mStateMachine->AddTransition(new StateTransition(GoOverState, WaitingState,
		[&]() -> bool
		{
			return this->mCooldown == 0;
		}
	));

}

PickupGameObject::~PickupGameObject() {
	delete mStateMachine;
}

void PickupGameObject::Update(float dt) {
	mStateMachine->Update(dt);
}

void PickupGameObject::ChangeToRandomPickup()
{
	std::mt19937 rng(*mRandomSeed);
	std::bernoulli_distribution bool_distribution(0.5);
	mIsBuff = bool_distribution(rng);
	if (mIsBuff)
		mCurrentBuff = mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->GetRandomBuffFromPool(*mRandomSeed);
	else
		mCurrentItem = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetRandomItemFromPool(*mRandomSeed);
}

void PickupGameObject::ActivatePickup(int playerNo)
{
	if (mIsBuff)
		mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->ApplyBuffToPlayer(mCurrentBuff, playerNo);
	else
		mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->AddItemToPlayer(mCurrentItem, playerNo);

	mCooldown = INT_MAX;
}

void PickupGameObject::OnCollisionBegin(GameObject* otherObject)
{
	if (mCooldown == 0)
	{
		if (otherObject->GetIsPlayer())
			return;
		//ActivatePickup((*mPlayerObjectToPlayerNoMap)[otherObject]);
		ActivatePickup(0);
	}
}

void PickupGameObject::GoOver(float dt) {
	Vector3 OverPos = GetTransform().GetPosition() + Vector3(0, 10, 0);
	GetTransform().SetPosition(OverPos);
	mCooldown = 0;
}

void PickupGameObject::GoUnder(float dt) {
	Vector3 UnderPos = GetTransform().GetPosition() + Vector3(0, -10, 0);
	GetTransform().SetPosition(UnderPos);
	mCooldown = mInitCooldown;
}

void PickupGameObject::Waiting(float dt)
{
	mCooldown = std::max(mCooldown - dt, 0.0f);
}

