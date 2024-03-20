#pragma once
#include "Door.h"
#include "Interactable.h"
#include "../CSC8503/SuspicionSystem/GlobalSuspicionMetre.h"
namespace NCL {
	namespace CSC8503 {
		class InteractableDoor : public Door, public Interactable, public SuspicionSystem::GlobalSuspicionObserver {
		public:
			InteractableDoor();
			~InteractableDoor() {

			}

			void Unlock();
			void Lock();
			void Interact(InteractType interactType, GameObject* interactedObject = nullptr) override;
			bool CanBeInteractedWith(InteractType interactType, GameObject* interactedObject = nullptr) override;
#ifdef USEGL
			void SyncInteractableDoorStatusInMultiplayer(bool toOpen);
			void SyncDoor(bool toOpen);
#endif
			void CountDownLockTimer(float dt);

			virtual void UpdateObject(float dt) override;
			virtual void UpdateGlobalSuspicionObserver(SuspicionSystem::SuspicionMetre::SusBreakpoint susBreakpoint) override;
		protected:
			bool CanUseItem(GameObject* userObj);
			const float initDoorTimer = 5.0f;
			const float initLockCooldown = 2.0f;
			float mLockCooldown;
		};
	}
}
