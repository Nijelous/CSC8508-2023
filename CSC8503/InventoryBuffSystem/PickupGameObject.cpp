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
	bool isMultiplayer,
	unsigned int randomSeed,
	std::map<GameObject*, int>* playerObjectToPlayerNoMap,
	float initCooldown) {
	mPlayerObjectToPlayerNoMap = playerObjectToPlayerNoMap;
	mRandomSeed = randomSeed;
	mCooldown = 0.0f;
	mInitCooldown = initCooldown;
	mInventoryBuffSystemClassPtr = inventoryBuffSystemClassPtr;
	mPlayerObjectToPlayerNoMap = playerObjectToPlayerNoMap;
	mRandomSeed = randomSeed;
	mIsMultiplayer = isMultiplayer;
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
	ChangeToRandomPickup();
}

PickupGameObject::~PickupGameObject() {
	delete mStateMachine;
}

void PickupGameObject::UpdateObject(float dt) {
	mStateMachine->Update(dt);
}

void PickupGameObject::ChangeToRandomPickup(){
	std::random_device rd;
	std::mt19937 gen(rd());
	std::bernoulli_distribution d(0.5);
	mIsBuff = d(gen);
	if (mIsBuff)
		mCurrentBuff = mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->GetRandomBuffFromPool(mRandomSeed, !mIsMultiplayer);
	else
		mCurrentItem = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetRandomItemFromPool(mRandomSeed, !mIsMultiplayer);
}

void PickupGameObject::ActivatePickup(int playerNo){
	if (mIsBuff)
		mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->ApplyBuffToPlayer(mCurrentBuff, playerNo);
	else
		mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->AddItemToPlayer(mCurrentItem, playerNo);

	mCooldown = INT_MAX;
}

void PickupGameObject::OnCollisionBegin(GameObject* otherObject){
	if (mCooldown == 0)
	{
		//ActivatePickup((*mPlayerObjectToPlayerNoMap)[otherObject]);
		if (mIsBuff ||
			!mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->IsInventoryFull(0))
			ActivatePickup(0);
	}
}

void PickupGameObject::Activate(float dt) {
	SetActive(true);
	mCooldown = 0;
}

void PickupGameObject::Deactivate(float dt) {
	SetActive(false);
	mCooldown = mInitCooldown;
	ChangeToRandomPickup();
}

void PickupGameObject::Waiting(float dt){
	mCooldown = std::max(mCooldown - dt, 0.0f);
}

