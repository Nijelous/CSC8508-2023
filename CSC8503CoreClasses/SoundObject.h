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
			bool mIsTriggered;
			bool mIsClosed;
			bool mIsLocked;
		};
    }
}