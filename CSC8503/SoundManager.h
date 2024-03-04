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
			FMOD::Channel* AddDoorOpenSound();

			void UpdateSounds(vector<GameObject*> object);

			void UpdateSound(GameObject* obj);

			void UpdateFootstepSounds(GameObject::GameObjectState state, Vector3 soundPos, FMOD::Channel* channel);

			void UpdateOpenDoorSound(bool isOpen, Vector3 soundPos, FMOD::Channel* channel);

			void SetListenerAttributes();

			FMOD_VECTOR ConvertVector(Vector3 vector);

			FMOD_VECTOR GetUpVector(Vector3 forward, Vector3 right);

		protected:
			GameWorld* mGameWorld;
			FMOD::System* mSystem = nullptr;
			FMOD::Sound* mFootStepSound = nullptr;
			FMOD::Sound* mDoorOpenSound = nullptr;
			FMOD_RESULT mResult;
			bool mTempIsOpen = false;
		};
	}
}