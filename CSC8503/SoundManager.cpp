#include "SoundManager.h"
#include "Vector3.h"

using namespace NCL::CSC8503;
using namespace NCL::Maths;

SoundManager::SoundManager() {
	mSoundEngine = createIrrKlangDevice();
	footStep = mSoundEngine->play3D("", vec3df(0, 0, 0), true, true);
}

SoundManager::~SoundManager() {
	if (footStep) {
		footStep->drop();
	}
	mSoundEngine->drop();
}

ISound* SoundManager::GetSound() {
	/*mSound = FootStep;
	switch (mSound) {
	case FootStep:
		return footStep;
		break;
	case :
		break;
	}*/
	return footStep;
}

vec3df SoundManager::ConvertToVec3df(Vector3 soundPos) {
	return vec3df(soundPos.x, soundPos.y, soundPos.z);
}

void SoundManager::UpdateSound(ISound* sound, Vector3 soundPos, bool isPaused) {
	if (sound) {
		sound->setPosition(ConvertToVec3df(soundPos));
		sound->setIsPaused(isPaused);
	}
}