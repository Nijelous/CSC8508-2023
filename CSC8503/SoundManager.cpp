#include "SoundManager.h"
#include "Vector3.h"

using namespace NCL::CSC8503;
using namespace NCL::Maths;

SoundManager::SoundManager(GameWorld* GameWorld) {
	mGameWorld = GameWorld;
	//FMOD_RESULT result;
	FMOD::Sound* sound;
	//FMOD::Channel* channel;
	result = FMOD::System_Create(&system);

	if (result != FMOD_OK) {
		return;
	}

	result = system->init(512, FMOD_INIT_NORMAL, 0);

	if (result != FMOD_OK) {
		return;
	}

	//system->createSound("../Assets/Sounds/ophelia.mp3", FMOD_3D, 0, &sound);

	result = system->createSound("../Assets/Sounds/Barefoot-Footsteps-Fast-www.fesliyanstudios.com.mp3", FMOD_3D | FMOD_LOOP_NORMAL, 0, &footStepSound);

	if (result != FMOD_OK) {
		return;
	}


	//footStepSound->setMode(FMOD_3D);

	//system->playSound(sound, 0, false, &channel);

	//channel->setPaused(true);
}

SoundManager::~SoundManager() {
	footStepSound->release();
	system->close();
	system->release();
}

FMOD::Channel* SoundManager::AddWalkSound(Vector3 soundPos) {
	FMOD::Channel* footStepChannel;
	result = system->playSound(footStepSound, 0, false, &footStepChannel);
	if (result != FMOD_OK) {
		return nullptr;
	}
	FMOD_VECTOR pos = ConvertVector(soundPos);
	footStepChannel->set3DAttributes(&pos, nullptr);
	return footStepChannel;
}

//void SoundManager::UpdateSounds(GameObject::GameObjectState state, Vector3 soundPos) {
void SoundManager::UpdateSounds(PlayerObject::PlayerState state, Vector3 soundPos) {
	//FMOD::Channel* channel = nullptr;
	SetListenerAttributes();
	/*switch (state) {
	case PlayerObject::PlayerState::Stand:
		if (channel) {
			channel->setPaused(true);
		}
		break;
	case PlayerObject::PlayerState::Walk:
		if (channel) {
			channel->setPaused(false);
		}
		else {
			channel = AddWalkSound(soundPos);
		}
		break;
	case PlayerObject::PlayerState::Sprint:
		if (channel) {
			channel->setPaused(false);
		}
		else {
			channel = AddWalkSound(soundPos);
		}
		break;
	case PlayerObject::PlayerState::Crouch:
		if (channel) {
			channel->setPaused(true);
		}
		break;
	case PlayerObject::PlayerState::Happy:
		break;
	}*/
	/*switch (state) {
	case GameObject::GameObjectState::Stand:
		if (channel) {
			channel->setPaused(true);
		}
		break;
	case GameObject::GameObjectState::Walk:
		if (channel) {
			channel->setPaused(false);
		}
		else {
			channel = AddWalkSound(soundPos);
		}
		break;
	case GameObject::GameObjectState::Sprint:
		if (channel) {
			channel->setPaused(false);
		}
		else {
			channel = AddWalkSound(soundPos);
		}
		break;
	case GameObject::GameObjectState::Crouch:
		if (channel) {
			channel->setPaused(true);
		}
		break;
	case GameObject::GameObjectState::Happy:
		break;
	}*/
}

void SoundManager::SetListenerAttributes() {
	FMOD_VECTOR camPos = ConvertVector(mGameWorld->GetMainCamera().GetPosition());
	Vector3 forward = mGameWorld->GetMainCamera().GetForwardVector();
	Vector3 right = mGameWorld->GetMainCamera().GetRightVector();
	FMOD_VECTOR camForward = ConvertVector(forward);
	FMOD_VECTOR camUp = GetUpVector(forward, right);

	system->set3DListenerAttributes(0, &camPos, 0, &camForward, &camUp);
}
//
//ISound* SoundManager::AddSprintSound() {
//	ISound* run = mSoundEngine->play3D("../Assets/Sounds/Barefoot-Footsteps-Fast-www.fesliyanstudios.com.mp3", vec3df(0, 0, 0), true, true);
//	mSounds.emplace_back(run);
//	return run;
//}
//
//void SoundManager::PlayOneTimeSound(Vector3 position) {
//	ISound* oneTimeSound = mSoundEngine->play3D("soundFile.mp3", ConvertToVec3df(position));
//	mSounds.emplace_back(oneTimeSound);
//}
//
//void SoundManager::SetSoundToBePaused(ISound* sound, bool isPaused) {
//	sound->setIsPaused(isPaused);
//}
//
//void SoundManager::SetSoundPosition(ISound* sound, Vector3 pos) {
//	sound->setPosition(ConvertToVec3df(pos));
//}
//
//void SoundManager::DeleteSounds() {
//	for (int i = 0; i < mSounds.size(); i++) {
//		if (mSounds[i]) {
//			mSounds[i]->drop();
//		}
//	}
//}
//
FMOD_VECTOR SoundManager::ConvertVector(Vector3 vector) {
	FMOD_VECTOR position = { vector.x, vector.y, vector.z };
	return position;
}

FMOD_VECTOR SoundManager::GetUpVector(Vector3 forward, Vector3 right) {
	Vector3 up = Vector3::Cross(forward, right);
	FMOD_VECTOR upVector = { up.x, up.y, up.z };
	return upVector;
}
