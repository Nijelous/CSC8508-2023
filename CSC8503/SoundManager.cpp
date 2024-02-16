#include "SoundManager.h"
#include "Vector3.h"

using namespace NCL::CSC8503;
using namespace NCL::Maths;

SoundManager::SoundManager() {
	mSoundEngine = createIrrKlangDevice();
	mFootStep = mSoundEngine->play3D("", vec3df(0, 0, 0), true, true);
	mPickupSound = mSoundEngine->play3D("", vec3df(0, 0, 0), false, true);
}

SoundManager::~SoundManager() {
	if (mFootStep) {
		mFootStep->drop();
	}
	mSoundEngine->drop();
}

//ISound* SoundManager::GetSound(std::string soundName) {
//	/*mSound = FootStep;
//	switch (mSound) {
//	case FootStep:
//		return footStep;
//		break;
//	case :
//		break;
//	}*/
//	return footStep;
//}

void SoundManager::UpdateSound(int sound, Vector3 position, bool isPaused) {
	switch (sound) {
	case FootStep:
		if (mFootStep) {
			mFootStep->setPosition(ConvertToVec3df(position));
			mFootStep->setIsPaused(isPaused);
		}
	case PickupSound:
		if (mPickupSound) {
			if (mPickupSound->isFinished()) {
				mPickupSound->stop();
				mPickupSound = mSoundEngine->play3D("", ConvertToVec3df(position), false, false);
			}
			else {
				mPickupSound->setPosition(ConvertToVec3df(position));
				mPickupSound->setIsPaused(isPaused);
			}
		}
	}

}

vec3df SoundManager::ConvertToVec3df(Vector3 soundPos) {
	return vec3df(soundPos.x, soundPos.y, soundPos.z);
}

//void SoundManager::UpdateSound(ISound* sound, Vector3 soundPos, bool isPaused) {
//	if (sound) {
//		sound->setPosition(ConvertToVec3df(soundPos));
//		sound->setIsPaused(isPaused);
//	}
//}