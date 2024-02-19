#pragma once
#include "Door.h"
#include "Interactable.h"
#include "../CSC8503/SuspicionSystem/GlobalSuspicionMetre.h"
namespace NCL {
	namespace CSC8503 {
		class InteractableDoor : public Door, Interactable, SuspicionSystem::GlobalSuspicionObserver {
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

			virtual void UpdateGlobalSuspicionObserver(SuspicionSystem::SuspicionMetre::SusBreakpoint susBreakpoint) override;

		protected:
			const float initDoorTimer = 7.0f;
			bool mIsLocked;
		};
	}
}
