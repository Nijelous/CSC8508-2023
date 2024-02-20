#pragma once
#include "Door.h"
#include "Interactable.h"
#include "../CSC8503/SuspicionSystem/GlobalSuspicionMetre.h"
namespace NCL {
	namespace CSC8503 {
		class InteractableDoor : public Door, public Interactable, SuspicionSystem::GlobalSuspicionObserver {
		public:
			InteractableDoor(){
				GameObject::mName = "InteractableDoor";
				mIsLocked = false;
				mIsOpen = false;
				InitStateMachine();
				//mRelatedItem = InventoryBuffSystem::PlayerInventory::doorKey;
			}

			void Unlock();
			void Lock();
			void Interact(InteractType interactType) override;
			bool CanBeInteractedWith(InteractType interactType) override;
			virtual void InitStateMachine() override;

			virtual void UpdateObject(float dt);
			virtual void UpdateGlobalSuspicionObserver(SuspicionSystem::SuspicionMetre::SusBreakpoint susBreakpoint) override;

		protected:
			const float initDoorTimer = 3.0f;
			bool mIsLocked;
		};
	}
}
