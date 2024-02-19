#include "SoundManager.h"
#include "Vector3.h"

using namespace NCL::CSC8503;
using namespace NCL::Maths;

SoundManager::SoundManager() {
	mSoundEngine = createIrrKlangDevice();
	//mFootStep = mSoundEngine->play3D("", vec3df(0, 0, 0), true, true);
}

SoundManager::~SoundManager() {
	/*if (mFootStep) {
		mFootStep->drop();
	}*/
	DeleteSounds();
	mSoundEngine->drop();
}

ISound* SoundManager::AddFootStepSound(Vector3 soundPos) {
	ISound* footStep = mSoundEngine->play3D("", ConvertToVec3df(soundPos), true);
	mSounds.emplace_back(footStep);
	return footStep;
}

void SoundManager::PlayOneTimeSound(Vector3 position) {
	ISound* oneTimeSound = mSoundEngine->play3D("soundFile.mp3", ConvertToVec3df(position));
}

void SoundManager::SetSoundPauseState(ISound* sound, bool isPaused) {
	sound->setIsPaused(isPaused);
}

void SoundManager::SetSoundPosition(ISound* sound, Vector3 pos) {
	sound->setPosition(ConvertToVec3df(pos));
}

void SoundManager::DeleteSounds() {
	for (int i = 0; i < mSounds.size(); i++) {
		if (mSounds[i]) {
			mSounds[i]->drop();
		}
	}
}

vec3df SoundManager::ConvertToVec3df(Vector3 soundPos) {
	return vec3df(soundPos.x, soundPos.y, soundPos.z);
}
