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

			ISound* GetSound();
			vec3df ConvertToVec3df(Vector3 soundPos);

			void UpdateSound(ISound* sound, Vector3 soundPos, bool isPaused);
			ISound* footStep;

		protected:
			//ISound* footStep;
			ISoundEngine* soundEngine;
		};
	}
}