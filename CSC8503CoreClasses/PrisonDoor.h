#pragma once
#include "Door.h"
#include "../CSC8503/SuspicionSystem/GlobalSuspicionMetre.h"
namespace NCL {
	namespace CSC8503 {
		class PrisonDoor : public Door, public SuspicionSystem::GlobalSuspicionObserver{
		public:
			PrisonDoor() {
				mName = "Prison Door";
				mTimer = initDoorTimer;
				mIsOpen = false;
			}

			virtual void UpdateGlobalSuspicionObserver(SuspicionSystem::SuspicionMetre::SusBreakpoint susBreakpoint) override;
			virtual void Open() override;
			virtual void Close() override;
			virtual void UpdateObject(float dt) override;

			virtual void SetIsOpen(bool toOpen) override;
#ifdef USEGL
			void SyncInteractableDoorStatusInMultiplayer(bool toOpen);
			void SyncDoor(bool toOpen);
#endif
		protected:
			const int initDoorTimer = 5;
		};
	}
}

