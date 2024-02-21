#include "SoundEmitter.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "Vector3.h"
#include "map";

using namespace NCL;
using namespace CSC8503;

SoundEmitter::SoundEmitter(float initCooldown, LocationBasedSuspicion* locationBasedSuspicionPTR) {

	mCooldown = initCooldown;
	mLocationBasedSuspicionPTR = locationBasedSuspicionPTR;

	NCL::Maths::Vector3 pos = GetTransform().GetPosition();

	mLocationBasedSuspicionPTR->AddActiveLocationSusCause(LocationBasedSuspicion::continouousSound, pos.x,pos.z);
}

SoundEmitter::~SoundEmitter() {
}

void SoundEmitter::UpdateObject(float dt) {
	mCooldown -= dt;

	if (mCooldown <= 0 && mIsActive)
	{
		NCL::Maths::Vector3 pos = GetTransform().GetPosition();
		mLocationBasedSuspicionPTR->RemoveActiveLocationSusCause(LocationBasedSuspicion::continouousSound, pos.x, pos.z);
		SetActive();
	}
}

