#include "SoundEmitter.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "Vector3.h"
#include "map"
#include "SoundObject.h"

using namespace NCL;
using namespace CSC8503;

SoundEmitter::SoundEmitter(float initCooldown, LocationBasedSuspicion* locationBasedSuspicionPTR, const Vector3& position ) {

	mCooldown = initCooldown;
	mLocationBasedSuspicionPTR = locationBasedSuspicionPTR;

	mLocationBasedSuspicionPTR->AddActiveLocationSusCause(LocationBasedSuspicion::continouousSound, position);
}

SoundEmitter::~SoundEmitter() {
}

void SoundEmitter::UpdateObject(float dt) {
	mCooldown -= dt;

	if (mCooldown <= 0 && this->IsActive())
	{
		NCL::Maths::Vector3 pos = GetTransform().GetPosition();
		mLocationBasedSuspicionPTR->RemoveActiveLocationSusCause(LocationBasedSuspicion::continouousSound, pos);
		this->SetActive(false);
#ifdef USEGL
		this->GetSoundObject()->GetChannel()->setPaused(true);
#endif
	}
}

