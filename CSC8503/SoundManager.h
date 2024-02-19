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

			vec3df ConvertToVec3df(Vector3 soundPos);
			
			ISound* AddFootStepSound(Vector3 position);

			void PlayOneTimeSound(Vector3 position);

			void SetSoundPauseState(ISound* sound, bool isPause);

			void SetSoundPosition(ISound* sound, Vector3 pos);

			void DeleteSounds();

		protected:
			//ISound* mFootStep;
			std::vector<ISound*> mSounds;
			ISoundEngine* mSoundEngine;
		};
	}
}