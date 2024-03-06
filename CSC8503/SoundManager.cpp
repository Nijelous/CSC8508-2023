#include "SoundManager.h"
#include "Vector3.h"

using namespace NCL::CSC8503;
using namespace NCL::Maths;

SoundManager::SoundManager(GameWorld* GameWorld) {
	mGameWorld = GameWorld;
	mResult = FMOD::System_Create(&mSystem);

	if (mResult != FMOD_OK) {
		std::cout << "!! Sound System Create Error !!" << std::endl;
		return;
	}

	mResult = mSystem->init(512, FMOD_INIT_NORMAL, 0);

	if (mResult != FMOD_OK) {
		std::cout << "!! Sound System init Error !!" << std::endl;
		return;
	}

	mResult = mSystem->createSound("../Assets/Sounds/Footsteps-on-metal-warehouse-floor--Slow--www.fesliyanstudios.com.mp3", FMOD_3D | FMOD_LOOP_NORMAL, 0, &mFootStepSound);

	if (mResult != FMOD_OK) {
		std::cout<<"!! Create Footstep Sound Error !!" << std::endl;
		return;
	}

	mResult = mSystem->createSound("../Assets/Sounds/opening-door-1-www.FesliyanStudios.com.mp3", FMOD_3D, 0, &mDoorOpenSound);
	if (mResult != FMOD_OK) {
		std::cout << "!! Create Door Open Sound Error !!" << std::endl;
		return;
	}

	mResult = mSystem->createSound("../Assets/Sounds/closing-door-2-www.FesliyanStudios.com.mp3", FMOD_3D, 0, &mDoorCloseSound);
	if (mResult != FMOD_OK) {
		std::cout << "!! Create Door Close Sound Error !!" << std::endl;
		return;
	}

	mResult = mSystem->createSound("../Assets/Sounds/ophelia.mp3", FMOD_3D, 0, &mSoundEmitterSound);
	if (mResult != FMOD_OK) {
		std::cout << "!! Create Sound Emitter Sound Error !!" << std::endl;
		return;
	}

	mResult = mFootStepSound->set3DMinMaxDistance(10.0f, 100.0f);
	if (mResult != FMOD_OK) {
		std::cout<<"FootStep Sound Attenuation Setting error" << std::endl;
		return;
	}

	mResult = mDoorOpenSound->set3DMinMaxDistance(20.0f, 100.0f);
	if (mResult != FMOD_OK) {
		std::cout << "Door Open Sound Attenuation Setting error" << std::endl;
		return;
	}

	mResult = mDoorCloseSound->set3DMinMaxDistance(20.0f, 100.0f);
	if (mResult != FMOD_OK) {
		std::cout << "Door Close Sound Attenuation Setting error" << std::endl;
		return;
	}
}

SoundManager::~SoundManager() {
	mDoorOpenSound->release();
	mDoorCloseSound->release();
	mFootStepSound->release();
	mSoundEmitterSound->release();
	mSystem->close();
	mSystem->release();
}

FMOD::Channel* SoundManager::AddWalkSound() {
	FMOD::Channel* footStepChannel;
	mResult = mSystem->playSound(mFootStepSound, 0, true, &footStepChannel);
	if (mResult != FMOD_OK) {
		std::cout << "Play Footstep sound error" << std::endl;
		return nullptr;
	}
	footStepChannel->setVolume(2.0f);
	return footStepChannel;
}

void SoundManager::PlayDoorOpenSound(Vector3 soundPos) {
	FMOD::Channel* doorOpenChannel;
	FMOD_VECTOR pos = ConvertVector(soundPos);
	mResult = mSystem->playSound(mDoorOpenSound, 0, true, &doorOpenChannel);
	if (mResult != FMOD_OK) {
		std::cout << "Play Door Open sound error" << std::endl;
		return;
	}
	mResult = doorOpenChannel->set3DAttributes(&pos, nullptr);
	if (mResult != FMOD_OK) {
		std::cout << "Play Door Open position setting error" << std::endl;
		return;
	}
	doorOpenChannel->setPaused(false);
}

void SoundManager::PlayDoorCloseSound(Vector3 soundPos) {
	FMOD::Channel* doorCloseChannel;
	FMOD_VECTOR pos = ConvertVector(soundPos);
	mResult = mSystem->playSound(mDoorCloseSound, 0, true, &doorCloseChannel);
	if (mResult != FMOD_OK) {
		std::cout << "Play Door Close sound error" << std::endl;
		return;
	}
	mResult = doorCloseChannel->set3DAttributes(&pos, nullptr);
	if (mResult != FMOD_OK) {
		std::cout << "Play Door Close position setting error" << std::endl;
		return;
	}
	doorCloseChannel->setPaused(false);
}

void SoundManager::UpdateSounds(vector<GameObject*> objects) {
	UpdateListenerAttributes();
	for (GameObject* obj : objects) {
		Vector3 soundPos = obj->GetTransform().GetPosition();
		if (obj->GetCollisionLayer() == CollisionLayer::Player) {
			GameObject::GameObjectState state = obj->GetGameOjbectState();
			FMOD::Channel* channel = obj->GetSoundObject()->GetChannel();
			UpdateFootstepSounds(state, soundPos, channel);
		}
		else if (obj->GetName() == "Guard") {
			GameObject::GameObjectState state = obj->GetGameOjbectState();
			FMOD::Channel* channel = obj->GetSoundObject()->GetChannel();
			UpdateFootstepSounds(state, soundPos, channel);
		}
#ifdef USEGL
		else if (Door* doorObj = dynamic_cast<Door*>(obj)) {
			bool isOpen = obj->GetSoundObject()->GetisTiggered();
			if (isOpen) {
				PlayDoorOpenSound(soundPos);
				obj->GetSoundObject()->SetNotTriggered();
			}
			bool isClose = obj->GetSoundObject()->GetIsClosed();
			if (isClose) {
				PlayDoorCloseSound(soundPos);
				obj->GetSoundObject()->CloseDoorFinished();
			}
		}
#endif
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
			channel->set3DAttributes(&pos, nullptr);
			channel->setPaused(false);
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

void SoundManager::UpdateListenerAttributes() {
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
