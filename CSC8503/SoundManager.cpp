#include "SoundManager.h"

using namespace NCL::CSC8503;
using namespace NCL::Maths;

#ifdef USEGL
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

	mResult = mSystem->createSound("../Assets/Sounds/ophelia.mp3", FMOD_3D | FMOD_LOOP_NORMAL, 0, &mSoundEmitterSound);
	if (mResult != FMOD_OK) {
		std::cout << "!! Create Sound Emitter Sound Error !!" << std::endl;
		return;
	}

	mResult = mSystem->createSound("../Assets/Sounds/item-pick-up-38258.mp3", FMOD_3D, 0, &mPickUpSound);
	if (mResult != FMOD_OK) {
		std::cout << "!! Create Pick Up Sound Error !!" << std::endl;
		return;
	}

	mResult = mSystem->createSound("../Assets/Sounds/door-lock-82542.mp3", FMOD_3D, 0, &mLockDoorSound);
	if (mResult != FMOD_OK) {
		std::cout << "!! Create Lock Door Sound Error !!" << std::endl;
		return;
	}

	mResult = mSystem->createSound("../Assets/Sounds/emergency-alarm-with-reverb-29431.mp3", FMOD_3D, 0, &mAlarmSound);
	if (mResult != FMOD_OK) {
		std::cout << "!! Create Alarm Sound Error !!" << std::endl;
		return;
	}

	mResult = mSystem->createSound("../Assets/Sounds/heater-vent-hit-higher-part-103305.mp3", FMOD_3D, 0, &mVentSound);
	if (mResult != FMOD_OK) {
		std::cout << "!! Create Vent Sound Error !!" << std::endl;
		return;
	}

	mResult = mSystem->createSound("../Assets/Sounds/warning-sound-6686.mp3", FMOD_2D, 0, &mSpottedSound);
	if (mResult != FMOD_OK) {
		std::cout << "!! Create Spotted Sound Error !!" << std::endl;
		return;
	}

	mResult = mSystem->createSound("../Assets/Sounds/Alarm-Fast-A1-www.fesliyanstudios.com.mp3", FMOD_3D | FMOD_LOOP_NORMAL, 0, &mCCTVSpotSound);
	if (mResult != FMOD_OK) {
		std::cout << "!! Create CCTV Spotted Sound Error !!" << std::endl;
		return;
	}

	mResult = mSystem->createSound("../Assets/Sounds/Metal-Pipe-Hit-Drop-A1-www.fesliyanstudios.com.mp3", FMOD_3D, 0, &mUnlockVentSound);
	if (mResult != FMOD_OK) {
		std::cout << "!! Create Unlock Vent Sound Error !!" << std::endl;
		return;
	}

	mResult = mFootStepSound->set3DMinMaxDistance(5.0f, 6000.0f);
	if (mResult != FMOD_OK) {
		std::cout<<"FootStep Sound Attenuation Setting error" << std::endl;
		return;
	}

	mResult = mDoorOpenSound->set3DMinMaxDistance(3.0f, 6000.0f);
	if (mResult != FMOD_OK) {
		std::cout << "Door Open Sound Attenuation Setting error" << std::endl;
		return;
	}

	mResult = mDoorCloseSound->set3DMinMaxDistance(3.0f, 6000.0f);
	if (mResult != FMOD_OK) {
		std::cout << "Door Close Sound Attenuation Setting error" << std::endl;
		return;
	}

	mResult = mSoundEmitterSound->set3DMinMaxDistance(12.0f, 650.0f);
	if (mResult != FMOD_OK) {
		std::cout << "Sound Emitter Sound Attenuation Setting error" << std::endl;
		return;
	}

	mResult = mAlarmSound->set3DMinMaxDistance(20.0f, 600.0f);
	if (mResult != FMOD_OK) {
		std::cout << "Alarm Sound Attenuation Setting error" << std::endl;
		return;
	}

	mResult = mCCTVSpotSound->set3DMinMaxDistance(15.0f, 1800.0f);
	if (mResult != FMOD_OK) {
		std::cout << "CCTV Attenuation Setting error" << std::endl;
		return;
	}

}

SoundManager::~SoundManager() {
	mDoorOpenSound->release();
	mDoorCloseSound->release();
	mFootStepSound->release();
	mSoundEmitterSound->release();
	mPickUpSound->release();
	mLockDoorSound->release();
	mAlarmSound->release();
	mVentSound->release();
	mSpottedSound->release();
	mCCTVSpotSound->release();
	mUnlockVentSound->release();
	mSystem->close();
	mSystem->release();
}

FMOD::Channel* SoundManager::AddWalkSound() {
	FMOD::Channel* footStepChannel = nullptr;
	mResult = mSystem->playSound(mFootStepSound, 0, true, &footStepChannel);
	if (mResult != FMOD_OK) {
		std::cout << "Play Footstep sound error" << std::endl;
		return nullptr;
	}
	mResult = footStepChannel->setVolume(2.0f);
	if (mResult != FMOD_OK) {
		std::cout << "Footstep sound Volume Change error" << std::endl;
		return nullptr;
	}
	return footStepChannel;
}

FMOD::Channel* SoundManager::AddSoundEmitterSound(Vector3 soundPos) {
	FMOD::Channel* soundEmitterChannel = nullptr;
	FMOD_VECTOR pos = ConvertVector(soundPos);
	mResult = mSystem->playSound(mSoundEmitterSound, 0, true, &soundEmitterChannel);
	if (mResult != FMOD_OK) {
		std::cout << "Play Sound Emitter sound error" << std::endl;
		return nullptr;
	}
	mResult = soundEmitterChannel->set3DAttributes(&pos, nullptr);
	if (mResult != FMOD_OK) {
		std::cout << "Sound Emitter position setting error" << std::endl;
		return nullptr;
	}
	mSystem->update();
	soundEmitterChannel->setPaused(false);
	return soundEmitterChannel;
}

FMOD::Channel* SoundManager::AddCCTVSpotSound() {
	FMOD::Channel* CCTVSpotChannel = nullptr;
	mResult = mSystem->playSound(mCCTVSpotSound, 0, true, &CCTVSpotChannel);
	if (mResult != FMOD_OK) {
		std::cout << "Play CCTV Spot sound error" << std::endl;
		return nullptr;
	}
	mSystem->update();
	return CCTVSpotChannel;
}

void SoundManager::PlayDoorOpenSound(Vector3 soundPos) {
	FMOD::Channel* doorOpenChannel = nullptr;
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
	FMOD::Channel* doorCloseChannel = nullptr;
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

void SoundManager::PlayPickUpSound(Vector3 soundPos) {
	FMOD::Channel* pickUpChannel = nullptr;
	FMOD_VECTOR pos = ConvertVector(soundPos);
	mResult = mSystem->playSound(mPickUpSound, 0, true, &pickUpChannel);
	if (mResult != FMOD_OK) {
		std::cout << "Play Pick Up sound error" << std::endl;
		return;
	}
	mResult = pickUpChannel->set3DAttributes(&pos, nullptr);
	if (mResult != FMOD_OK) {
		std::cout << "Pick Up Sound position setting error" << std::endl;
		return;
	}
	pickUpChannel->setVolume(0.5f);
	pickUpChannel->setPaused(false);
}

void SoundManager::PlayLockDoorSound(Vector3 soundPos) {
	FMOD::Channel* lockDoorChannel = nullptr;
	FMOD_VECTOR pos = ConvertVector(soundPos);
	mResult = mSystem->playSound(mLockDoorSound, 0, true, &lockDoorChannel);
	if (mResult != FMOD_OK) {
		std::cout << "Play Lock Door sound error" << std::endl;
		return;
	}
	mResult = lockDoorChannel->set3DAttributes(&pos, nullptr);
	if (mResult != FMOD_OK) {
		std::cout << "Lock Door position setting error" << std::endl;
		return;
	}
	lockDoorChannel->setPaused(false);
}

void SoundManager::PlayAlarmSound(Vector3 soundPos) {
	FMOD::Channel* alarmChannel = nullptr;
	FMOD_VECTOR pos = ConvertVector(soundPos);
	mResult = mSystem->playSound(mAlarmSound, 0, true, &alarmChannel);
	if (mResult != FMOD_OK) {
		std::cout << "Play Alarm sound error" << std::endl;
		return;
	}
	mResult = alarmChannel->set3DAttributes(&pos, nullptr);
	if (mResult != FMOD_OK) {
		std::cout << "Alarm sound position setting error" << std::endl;
		return;
	}
	alarmChannel->setPaused(false);
}

void SoundManager::PlayVentSound(Vector3 soundPos) {
	Channel* channel = nullptr;
	FMOD_VECTOR pos = ConvertVector(soundPos);
	mResult = mSystem->playSound(mVentSound, 0, true, &channel);
	if (mResult != FMOD_OK) {
		std::cout << "Vent Sound error" << std::endl;
		return;
	}
	mResult = channel->setVolume(2.0f);
	if (mResult != FMOD_OK) {
		std::cout << "Vent Sound Volume setting error" << std::endl;
		return;
	}
	mResult = channel->set3DAttributes(&pos, nullptr);
	if (mResult != FMOD_OK) {
		std::cout << "Vent Sound position setting error" << std::endl;
		return;
	}
	channel->setPaused(false);
}

void SoundManager::PlayUnlockVentSound(Vector3 soundPos) {
	Channel* channel = nullptr;
	FMOD_VECTOR pos = ConvertVector(soundPos);
	mResult = mSystem->playSound(mUnlockVentSound, 0, true, &channel);
	if (mResult != FMOD_OK) {
		std::cout << "Unlock Vent Sound error" << std::endl;
		return;
	}
	mResult = channel->setVolume(2.0f);
	if (mResult != FMOD_OK) {
		std::cout << "Unlock Vent Sound Volume setting error" << std::endl;
		return;
	}
	mResult = channel->set3DAttributes(&pos, nullptr);
	if (mResult != FMOD_OK) {
		std::cout << "Unlock Vent Sound position setting error" << std::endl;
		return;
	}
	channel->setPaused(false);
}

void SoundManager::PlaySpottedSound() {
	Channel* channel;
	mResult = mSystem->playSound(mSpottedSound, 0, false, &channel);
	if (mResult != FMOD_OK) {
		std::cout << "Play Spotted Sound error" << std::endl;
		return;
	}
	mSystem->update();
}

void SoundManager::UpdateSounds(vector<GameObject*> objects) {
	UpdateListenerAttributes();
	for (GameObject* obj : objects) {
		Vector3 soundPos = obj->GetTransform().GetPosition();
		if (obj->GetCollisionLayer() == CollisionLayer::Player) {
			bool isClose = obj->GetSoundObject()->GetIsClosed();
			if (!isClose) {
				GameObject::GameObjectState state = obj->GetGameOjbectState();
				FMOD::Channel* channel = obj->GetSoundObject()->GetChannel();
				UpdateFootstepSounds(state, soundPos, channel);
			}
			bool isTrigger = obj->GetSoundObject()->GetisTiggered();
			if (isTrigger) {
				PlaySpottedSound();
				obj->GetSoundObject()->SetNotTriggered();
			}
		}
		else if (obj->GetName() == "Guard") {
			GameObject::GameObjectState state = obj->GetGameOjbectState();
			FMOD::Channel* channel = obj->GetSoundObject()->GetChannel();
			UpdateFootstepSounds(state, soundPos, channel);
		}
		else {
			bool isTrigger = obj->GetSoundObject()->GetisTiggered();
			if (obj->GetName() == "CCTV") {
				Channel* channel = obj->GetSoundObject()->GetChannel();
				UpdateCCTVSpotSound(isTrigger, soundPos, channel);
			}
			if (isTrigger) {
				if (obj->GetName() == "InteractableDoor") {
					PlayDoorOpenSound(soundPos);
				}
				else if (obj->GetName() == "PickupGameObject") {
					PlayPickUpSound(soundPos);
				}
				else if (obj->GetName() == "Flag") {
					PlayAlarmSound(soundPos);
				}
				else if (obj->GetName() == "Vent") {
					PlayVentSound(soundPos);
				}
				obj->GetSoundObject()->SetNotTriggered();
			}
			bool isClose = obj->GetSoundObject()->GetIsClosed();
			if (isClose && (obj->GetName() == "InteractableDoor")) {
				PlayDoorCloseSound(soundPos);
				obj->GetSoundObject()->CloseDoorFinished();
			}
			bool isLocked = obj->GetSoundObject()->GetIsLocked();
			if (isLocked) {
				if (obj->GetName() == "InteractableDoor") {
					PlayLockDoorSound(soundPos);
					obj->GetSoundObject()->LockDoorFinished();
				}
				else if (obj->GetName() == "Vent") {
					PlayUnlockVentSound(soundPos);
					obj->GetSoundObject()->LockDoorFinished();
				}
			}
		}
	}
	mSystem->update();
}

void SoundManager::UpdateFootstepSounds(GameObject::GameObjectState state, Vector3 soundPos, FMOD::Channel* channel) {

	FMOD_VECTOR pos = ConvertVector(soundPos);

	switch (state) {
	case GameObject::GameObjectState::Idle:
		channel->setPaused(true);
		break;
	case GameObject::GameObjectState::Walk:
		channel->set3DAttributes(&pos, nullptr);
		channel->setFrequency(48000);
		channel->setPaused(false);
		break;
	case GameObject::GameObjectState::Sprint:
		channel->set3DAttributes(&pos, nullptr);
		channel->setFrequency(96000);
		channel->setPaused(false);
		break;
	case GameObject::GameObjectState::IdleCrouch:
		channel->setPaused(true);
		break;
	case GameObject::GameObjectState::Crouch:
		channel->setPaused(true);
		break;
	case GameObject::GameObjectState::Happy:
		break;
	}
}

void SoundManager::UpdateCCTVSpotSound(bool isPlay, Vector3 soundPos, Channel* channel) {
	FMOD_VECTOR pos = ConvertVector(soundPos);
	if (isPlay) {
		channel->set3DAttributes(&pos, nullptr);
		channel->setPaused(false);
	}
	else {
		mResult = channel->setPaused(true);
	}
}

void SoundManager::UpdateListenerAttributes() {
	FMOD_VECTOR camPos = ConvertVector(mGameWorld->GetMainCamera().GetPosition());
	Vector3 forward = mGameWorld->GetMainCamera().GetForwardVector();
	Vector3 right = mGameWorld->GetMainCamera().GetRightVector();
	FMOD_VECTOR camForward = ConvertVector(forward);
	FMOD_VECTOR camUp = GetUpVector(forward, right);
	mSystem->set3DListenerAttributes(0, &camPos, 0, &camForward, &camUp);
	mSystem->update();
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
#endif