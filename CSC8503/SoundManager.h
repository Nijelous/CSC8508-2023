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

			enum Sounds{
				walk,
				run,
			};

			vec3df ConvertToVec3df(Vector3 soundPos);
			
			ISound* AddWalkSound();

			ISound* AddRunSound();

			void PlayOneTimeSound(Vector3 position);

			void SetSoundToBePaused(ISound* sound, bool isPaused);

			void SetSoundPosition(ISound* sound, Vector3 pos);

			void DeleteSounds();

		protected:
			//ISound* mFootStep;
			std::map<Sounds, std::function<ISound()>> mSoundMap;
			std::vector<ISound*> mSounds;
			ISoundEngine* mSoundEngine;
		};
	}
}