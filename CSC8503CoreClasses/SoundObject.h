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

			void CloseDoorTriggered();

			void LockDoorTriggered();

			bool GetisTiggered();

			bool GetIsClosed();

			bool GetIsLocked();

			void SetNotTriggered();

			void CloseDoorFinished();

			void LockDoorFinished();

		protected:
			Channel* mChannel = nullptr;
			std::vector<Channel*> mChannels;
			bool mIsTriggered;
			bool mIsClosed;
			bool mIsLocked;
		};
    }
}