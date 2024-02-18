#pragma once
#include "Door.h"
#include "Interactable.h"
//#include "../CSC8503/InventoryBuffSystem/PlayerInventory.h"
namespace NCL {
	namespace CSC8503 {
		class InteractableDoor : public Door, Interactable {
		public:
			InteractableDoor(){
				GameObject::mName = "InteractableDoor";
				mIsLocked = false;
				mRelatedItem = InventoryBuffSystem::PlayerInventory::doorKey;
			}

			void Unlock();
			void Lock();
			void Interact(InteractType interactType) override;
			bool CanBeInteractedWith(InteractType interactType) override;
			virtual void InitStateMachine();

		protected:
			const float initDoorTimer = 7.0f;
			bool mIsLocked;
		};
	}
}
