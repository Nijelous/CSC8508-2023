#pragma once
#include "Door.h"
#include "../CSC8503/SuspicionSystem/GlobalSuspicionMetre.h"
namespace NCL {
	namespace CSC8503 {
		class PrisonDoor : public Door, SuspicionSystem::GlobalSuspicionObserver {
		public:
			PrisonDoor() {
				mName = "Prison Door";
				InitStateMachine();
				mIsOpen = false;
			}

			virtual void UpdateGlobalSuspicionObserver(SuspicionSystem::SuspicionMetre::SusBreakpoint susBreakpoint) override;
			virtual void InitStateMachine() override;
			virtual void UpdateObject(float dt);

		protected:
		};
	}
}

