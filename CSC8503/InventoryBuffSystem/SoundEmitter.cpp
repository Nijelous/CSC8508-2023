#include "SoundEmitter.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "Vector3.h"
#include "map";

using namespace NCL;
using namespace CSC8503;

SoundEmitter::SoundEmitter(int initCooldown, LocationBasedSuspicion* locationBasedSuspicionPTR) {

	mInitCooldown = initCooldown;
	mLocationBasedSuspicionPTR = locationBasedSuspicionPTR;

	NCL::Maths::Vector3 pos = GetTransform().GetPosition();

	mLocationBasedSuspicionPTR->AddActiveLocationSusCause(LocationBasedSuspicion::continouousSound, pos.x,pos.z);
}

SoundEmitter::~SoundEmitter() {
}

void SoundEmitter::Update(float dt) {
	mInitCooldown -= dt;

	if (mInitCooldown < 0)
	{
		this->SetIsRendered(false);
		NCL::Maths::Vector3 pos = GetTransform().GetPosition();
		mLocationBasedSuspicionPTR->RemoveActiveLocationSusCause(LocationBasedSuspicion::continouousSound, pos.x, pos.z);
	}
}

