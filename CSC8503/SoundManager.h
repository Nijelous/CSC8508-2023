#pragma once
#include <irrKlang.h>
#include "Vector3.h"

using namespace irrklang;

namespace NCL{
	using namespace Maths;
	namespace CSC8503 {
		class SoundManager {
		public:
			SoundManager();
			~SoundManager();

			enum Sounds {
				FootStep,
				PickupSound,
			};

			//ISound* GetSound(std::string soundName);
			vec3df ConvertToVec3df(Vector3 soundPos);
			
			//void UpdateSound(ISound* sound, Vector3 soundPos, bool isPaused);
			
			void UpdateSound(int sound, Vector3 position, bool isPaused);

			ISound* mFootStep;

			ISound* mPickupSound;

		protected:
			//ISound* footStep;
			ISoundEngine* mSoundEngine;
		};
	}
}