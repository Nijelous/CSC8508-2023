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

	mResult = mSystem->createSound("../Assets/Sounds/Footsteps-on-metal-warehouse-floor--Slow--www.fesliyanstudios.com.mp3", FMOD_3D | FMOD_LOOP_NORMAL, 0, &mFootStepSound);

	if (mResult != FMOD_OK) {
		return;
	}

	mFootStepSound->set3DMinMaxDistance(10.0f, 100.0f);
	if (mResult != FMOD_OK) {
		std::cout<<"Attenuation Setting error" << std::endl;
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

FMOD::Channel* SoundManager::AddWalkSound() {
	FMOD::Channel* footStepChannel;
	mResult = mSystem->playSound(mFootStepSound, 0, true, &footStepChannel);
	if (mResult != FMOD_OK) {
		std::cout << "Add walk sound error" << std::endl;
		return nullptr;
	}
	footStepChannel->setVolume(2.0f);
	return footStepChannel;
}

void SoundManager::UpdateSounds(vector<GameObject*> objects) {
	SetListenerAttributes();
	for (GameObject* obj : objects) {
		Vector3 soundPos = obj->GetTransform().GetPosition();
		GameObject::GameObjectState state = obj->GetGameOjbectState();
		if (PlayerObject* playerObj = dynamic_cast<PlayerObject*>(obj)) {
			FMOD::Channel* channel = obj->GetSoundObject()->GetChannel();
			UpdateFootstepSounds(state, soundPos, channel);
		}
		else if (GuardObject* guardObj = dynamic_cast<GuardObject*>(obj)) {
			FMOD::Channel* channel = obj->GetSoundObject()->GetChannel();
			UpdateFootstepSounds(state, soundPos, channel);
		}
	}
	mSystem->update();
}

void SoundManager::UpdateFootstepSounds(GameObject::GameObjectState state, Vector3 soundPos, FMOD::Channel* channel) {

	FMOD_VECTOR pos = ConvertVector(soundPos);

	switch (state) {
	case GameObject::GameObjectState::Idle:
		if (channel) {
			channel->setPaused(true);
		}
		break;
	case GameObject::GameObjectState::Walk:
		if (channel) {
			channel->setPaused(false);
			channel->set3DAttributes(&pos, nullptr);
			//!!!Issue!!!  !!!!cannot hear sound uppaused if set posiiton before uspause the channel !!!!
		}
		break;
	case GameObject::GameObjectState::Sprint:
		if (channel) {
			channel->set3DAttributes(&pos, nullptr);
			channel->setPaused(false);
		}
		break;
	case GameObject::GameObjectState::IdleCrouch:
		if (channel) {
			channel->setPaused(true);
		}
		break;
	case GameObject::GameObjectState::Crouch:
		if (channel) {
			channel->setPaused(true);
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
