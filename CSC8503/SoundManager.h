#pragma once
#include "Vector3.h"
#include "../FMODCoreAPI/includes/fmod.hpp"
#include "../CSC8503CoreClasses/GameObject.h"
#include "../CSC8503CoreClasses/GuardObject.h"
#include "../CSC8503CoreClasses/GameWorld.h"
#include "../CSC8503CoreClasses/PlayerObject.h"
#include "SoundObject.h"

namespace NCL{
	using namespace Maths;
	namespace CSC8503 {
		class SoundManager {
		public:
			SoundManager(GameWorld* GameWorld);
			~SoundManager();
			
			FMOD::Channel* AddWalkSound();

			FMOD::Channel* AddSoundEmitterSound(Vector3 soundPos);

			void PlayDoorOpenSound(Vector3 soudPos);

			void PlayDoorCloseSound(Vector3 soundPos);

			void PlayPickUpSound(Vector3 soundPos);

			void UpdateSounds(vector<GameObject*> object);

			void UpdateFootstepSounds(GameObject::GameObjectState state, Vector3 soundPos, FMOD::Channel* channel);

			void UpdateListenerAttributes();

			FMOD_VECTOR ConvertVector(Vector3 vector);

			FMOD_VECTOR GetUpVector(Vector3 forward, Vector3 right);

		protected:
			GameWorld* mGameWorld;
			FMOD::System* mSystem = nullptr;
			FMOD::Sound* mFootStepSound = nullptr;
			FMOD::Sound* mDoorOpenSound = nullptr;
			FMOD::Sound* mDoorCloseSound = nullptr;
			FMOD::Sound* mSoundEmitterSound = nullptr;
			FMOD::Sound* mPickUpSound = nullptr;
			FMOD_RESULT mResult;
		};
	}
}