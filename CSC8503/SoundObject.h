#pragma once
#include "../FMODCoreAPI/includes/fmod.hpp"
#include <vector>


using namespace FMOD;
namespace NCL {
	namespace CSC8503 {
		class SoundObject {
		public:
			SoundObject(Channel* channel);
			SoundObject();
			~SoundObject();

			void AddChannel(Channel* channel);

			std::vector<Channel*> GetChannels();

			Channel* GetChannel();

			void TriggerSoundEvent();

			bool GetisTiggered();

			void SetNotTriggered();

			void SetChannel(Channel* channel);
		private:
			Channel* mChannel = nullptr;
			std::vector<Channel*> mChannels;
			bool mIsTriggered;
		};
    }
}