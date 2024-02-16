#pragma once
#include "Door.h"
#include "Interactable.h"

namespace NCL {
	namespace CSC8503 {
		class InteractableDoor : public Door, Interactable {
		public:
			InteractableDoor(){
				GameObject::mName = "InteractableDoor";
				mIsOpen = false;
				mIsLocked = false;
			}

			void Unlock();
			void Lock();
			void Interact() override;
			bool CanBeInteractedWith() override;

		protected:
			bool mIsLocked;
		};
	}
}
