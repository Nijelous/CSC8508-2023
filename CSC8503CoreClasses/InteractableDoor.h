#pragma once
#include "Door.h"
#include "Interactable.h"
#include "../CSC8503/SuspicionSystem/GlobalSuspicionMetre.h"
namespace NCL {
	namespace CSC8503 {
		class InteractableDoor : public Door, public Interactable, SuspicionSystem::GlobalSuspicionObserver {
		public:
			InteractableDoor();
			~InteractableDoor() {

			}

			void Unlock();
			void Lock();
			void Interact(InteractType interactType, GameObject* interactedObject = nullptr) override;
			bool CanBeInteractedWith(InteractType interactType) override;
			void SetIsOpen(bool isOpen, bool isSettedByServer);
			virtual void InitStateMachine() override;
			void SyncInteractableDoorStatusInMultiplayer();
			virtual void UpdateObject(float dt);
			virtual void UpdateGlobalSuspicionObserver(SuspicionSystem::SuspicionMetre::SusBreakpoint susBreakpoint) override;
		protected:
			bool CanUseItem();
			const float initDoorTimer = 60.0f;
			bool mIsLocked;
		};
	}
}
