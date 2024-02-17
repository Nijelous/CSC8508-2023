#pragma once
#include "Door.h"
#include "Interactable.h"

namespace NCL {
	namespace CSC8503 {
		class InteractableDoor : public Door, Interactable {
		public:
			InteractableDoor(){
				GameObject::mName = "InteractableDoor";
				mIsLocked = false;
			}

			void Unlock();
			void Lock();
			void Interact(InteractType interactType) override;
			bool CanBeInteractedWith(InteractType interactType) override;
			virtual void InitStateMachuine();

		protected:
			const float initDoorTimer = 7.0f;
			bool mIsLocked;
		};
	}
}
