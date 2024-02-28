#pragma once
#include <fmod.hpp>
#include <vector>

namespace NCL {
	namespace CSC8503 {
		class SoundObject {
		public:
			SoundObject(FMOD::Channel* channel);
			~SoundObject();

			void AddChannel();

			std::vector<FMOD::Channel> GetChannels();

		private:
			FMOD::Channel* mChannel;
			std::vector<FMOD::Channel> mChannels;
		};
    }
}