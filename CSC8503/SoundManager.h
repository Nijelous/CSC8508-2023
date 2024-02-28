#pragma once
#include "Vector3.h"
#include "../FMODCoreAPI/includes/fmod.hpp"
#include "../CSC8503CoreClasses/GameObject.h"
#include "../CSC8503CoreClasses/GuardObject.h"
#include "../CSC8503CoreClasses/GameWorld.h"
#include "../CSC8503CoreClasses/PlayerObject.h"

namespace NCL{
	using namespace Maths;
	namespace CSC8503 {
		class SoundManager {
		public:
			SoundManager(GameWorld* GameWorld);
			~SoundManager();
			
			FMOD::Channel* AddWalkSound(Vector3 soundPos);

			void UpdateSounds(vector<GameObject*> object);

			void UpdateFootstepSounds(GameObject::GameObjectState state, Vector3 soundPos);

			void SetListenerAttributes();

			FMOD_VECTOR ConvertVector(Vector3 vector);

			FMOD_VECTOR GetUpVector(Vector3 forward, Vector3 right);

		protected:
			GameWorld* mGameWorld;
			FMOD::System* mSystem = nullptr;
			FMOD::Sound* mFootStepSound = nullptr;
			FMOD_RESULT mResult;
			FMOD::Channel* mChannel = nullptr;
		};
	}
}