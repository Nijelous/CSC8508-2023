#pragma once
#include "Vector3.h"
#include "../CSC8503CoreClasses/SoundObject.h"
#include "../CSC8503CoreClasses/GameObject.h"
#include "../CSC8503CoreClasses/GameWorld.h"

namespace NCL{
	using namespace Maths;
	namespace CSC8503 {
		class SoundManager {
		public:
			SoundManager(GameWorld* GameWorld);
			~SoundManager();
			
			FMOD::Channel* AddWalkSound();

			FMOD::Channel* AddSoundEmitterSound(Vector3 soundPos);

			FMOD::Channel* AddCCTVSpotSound();

			void PlayDoorOpenSound(Vector3 soudPos);

			void PlayDoorCloseSound(Vector3 soundPos);

			void PlayPickUpSound(Vector3 soundPos);

			void PlayLockDoorSound(Vector3 soundPos);

			void PlayAlarmSound(Vector3 soundPos);

			void PlayVentSound(Vector3 soundpos);

			void PlayUnlockVentSound(Vector3 soundPos);

			void UpdateSounds(vector<GameObject*> object);

			void UpdateFootstepSounds(GameObject::GameObjectState state, Vector3 soundPos, FMOD::Channel* channel);

			void PlaySpottedSound();

			void UpdateCCTVSpotSound(bool isPlay, Vector3 soundPos, FMOD::Channel* channel);

			void UpdateListenerAttributes();

			FMOD_VECTOR ConvertVector(Vector3 vector);

			FMOD_VECTOR GetUpVector(Vector3 forward, Vector3 right);

		protected:
			GameWorld* mGameWorld = nullptr;
			FMOD::System* mSystem = nullptr;
			FMOD::Sound* mFootStepSound = nullptr;
			FMOD::Sound* mDoorOpenSound = nullptr;
			FMOD::Sound* mDoorCloseSound = nullptr;
			FMOD::Sound* mSoundEmitterSound = nullptr;
			FMOD::Sound* mPickUpSound = nullptr;
			FMOD::Sound* mLockDoorSound = nullptr;
			FMOD::Sound* mAlarmSound = nullptr;
			FMOD::Sound* mVentSound = nullptr;
			FMOD::Sound* mSpottedSound = nullptr;
			FMOD::Sound* mUnlockVentSound = nullptr;
			FMOD::Sound* mCCTVSpotSound = nullptr;
			FMOD_RESULT mResult;
		};
	}
}