#pragma once
#include "Door.h"
#include "../CSC8503/SuspicionSystem/GlobalSuspicionMetre.h"
namespace NCL {
	namespace CSC8503 {
		class PrisonDoor : public Door, public SuspicionSystem::GlobalSuspicionObserver{
		public:
			PrisonDoor() {
				mName = "Prison Door";
			}

			virtual void UpdateGlobalSuspicionObserver(SuspicionSystem::SuspicionMetre::SusBreakpoint susBreakpoint) override;
			virtual void Open() override;
			virtual void Close() override;
			virtual void UpdateObject(float dt) override;

		protected:
			const int initDoorTimer = 5;
		};
	}
}

