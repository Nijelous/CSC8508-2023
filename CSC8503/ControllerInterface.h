#pragma once

#include "Vector3.h"

#ifdef USEPROSPERO
#include "../PS5Core/PS5Window.h"
#endif

#include "Window.h"

namespace NCL {
	class ControllerInterface {
	public:

		ControllerInterface() {}
		~ControllerInterface() {}

		bool MoveForward();

		bool GetSelectPressed();

		bool GetInteractPressed();

		bool GetDropItemPressed();

		bool GetSwitchItemPressed();

		bool GetSprintDown();

		bool GetCrouchPressed();

#ifdef USEPROSPERO
		PS5::PS5Controller* GetPS5Controller() { return mPS5Controller; }

		void SetPS5Controller(PS5::PS5Controller* ps5Controller) { mPS5Controller = ps5Controller; }

	private:

		PS5::PS5Controller* mPS5Controller;
#endif
	};
}

