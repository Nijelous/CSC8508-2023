#include "SoundManager.h"
#include "Vector3.h"

#include <fmod.hpp>
using namespace NCL::CSC8503;
using namespace NCL::Maths;

SoundManager::SoundManager() {
	FMOD_RESULT result;
	FMOD::Sound* sound;
	FMOD::Channel* channel;
	result = FMOD::System_Create(&system);

	if (result != FMOD_OK) {
		return;
	}

	result = system->init(512, FMOD_INIT_NORMAL, 0);

	if (result != FMOD_OK) {
		return;
	}

	system->createSound("", FMOD_DEFAULT, 0, &sound);

	system->playSound(sound, 0, false, &channel);

	channel->setPaused(true);
}

SoundManager::~SoundManager() {
	system->close();
	system->release();
}

//ISound* SoundManager::AddWalkSound() {
//	ISound* walk = mSoundEngine->play3D("../Assets/Sounds/Barefoot-Footsteps-Fast-www.fesliyanstudios.com.mp3", vec3df(0, 0, 0), true, true);
//	mSounds.emplace_back(walk);
//	return walk;
//}
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
//vec3df SoundManager::ConvertToVec3df(Vector3 soundPos) {
//	return vec3df(soundPos.x, soundPos.y, soundPos.z);
//}
