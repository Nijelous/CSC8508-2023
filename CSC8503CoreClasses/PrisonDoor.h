#pragma once
#include "Door.h"
#include "../CSC8503/SuspicionSystem/GlobalSuspicionMetre.h"
namespace NCL {
	namespace CSC8503 {
		class PrisonDoor : public Door, SuspicionSystem::GlobalSuspicionObserver {
		public:
			PrisonDoor() {
				mName = "Prison Door";
			}

			virtual void UpdateGlobalSuspicionObserver(SuspicionSystem::SuspicionMetre::SusBreakpoint susBreakpoint) override;
		protected:
		};
	}
}

