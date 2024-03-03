#include "SoundManager.h"
#include "Vector3.h"

using namespace NCL::CSC8503;
using namespace NCL::Maths;

SoundManager::SoundManager(GameWorld* GameWorld) {
	mGameWorld = GameWorld;
	mResult = FMOD::System_Create(&mSystem);

	if (mResult != FMOD_OK) {
		return;
	}

	mResult = mSystem->init(512, FMOD_INIT_NORMAL, 0);

	if (mResult != FMOD_OK) {
		return;
	}

	mResult = mSystem->createSound("../Assets/Sounds/Barefoot-Footsteps-Fast-www.fesliyanstudios.com.mp3", FMOD_3D | FMOD_LOOP_NORMAL, 0, &mFootStepSound);

	if (mResult != FMOD_OK) {
		return;
	}


	//TO_DO
	//footStepSound->setMode(FMOD_3D);
}

SoundManager::~SoundManager() {
	mFootStepSound->release();
	mSystem->close();
	mSystem->release();
}

FMOD::Channel* SoundManager::AddWalkSound(Vector3 soundPos) {
	FMOD::Channel* footStepChannel;
	mResult = mSystem->playSound(mFootStepSound, 0, false, &footStepChannel);
	if (mResult != FMOD_OK) {
		return nullptr;
	}
	FMOD_VECTOR pos = ConvertVector(soundPos);
	footStepChannel->set3DAttributes(&pos, nullptr);
	return footStepChannel;
}

void SoundManager::UpdateSounds(GameObject::GameObjectState state, Vector3 soundPos) {
	FMOD_VECTOR pos = ConvertVector(soundPos);
	SetListenerAttributes();
	switch (state) {
	case GameObject::GameObjectState::Idle:
		if (mChannel) {
			mChannel->setPaused(true);
		}
		break;
	case GameObject::GameObjectState::Walk:
		if (mChannel) {
			mChannel->set3DAttributes(&pos, nullptr);
			mChannel->setPaused(false);
		}
		else {
			mChannel = AddWalkSound(soundPos);
		}
		break;
	case GameObject::GameObjectState::Sprint:
		if (mChannel) {
			mChannel->set3DAttributes(&pos, nullptr);
			mChannel->setPaused(false);
		}
		else {
			mChannel = AddWalkSound(soundPos);
		}
		break;
	case GameObject::GameObjectState::IdleCrouch:
		if (mChannel) {
			mChannel->setPaused(true);
		}
		break;
	case GameObject::GameObjectState::Crouch:
		if (mChannel) {
			mChannel->setPaused(true);
		}
		break;
	case GameObject::GameObjectState::Happy:
		break;
	}
}

void SoundManager::SetListenerAttributes() {
	FMOD_VECTOR camPos = ConvertVector(mGameWorld->GetMainCamera().GetPosition());
	Vector3 forward = mGameWorld->GetMainCamera().GetForwardVector();
	Vector3 right = mGameWorld->GetMainCamera().GetRightVector();
	FMOD_VECTOR camForward = ConvertVector(forward);
	FMOD_VECTOR camUp = GetUpVector(forward, right);

	mSystem->set3DListenerAttributes(0, &camPos, 0, &camForward, &camUp);
}

FMOD_VECTOR SoundManager::ConvertVector(Vector3 vector) {
	FMOD_VECTOR position = { vector.x, vector.y, vector.z };
	return position;
}

FMOD_VECTOR SoundManager::GetUpVector(Vector3 forward, Vector3 right) {
	Vector3 up = Vector3::Cross(forward, right);
	FMOD_VECTOR upVector = { up.x, up.y, up.z };
	return upVector;
}
