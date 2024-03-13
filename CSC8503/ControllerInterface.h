#pragma once

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
		bool MoveBackwards();
		bool MoveRight();
		bool MoveLeft();

		bool GetSelectPressed();

		bool GetInteractPressed();

		bool GetInteractHeld();

		bool GetDropItemPressed();

		bool CheckSwitchItemOne();

		bool CheckSwitchItemTwo();

		bool GetSprintDown();

		bool GetCrouchPressed();

		bool UseItemPressed();

#ifdef USEPROSPERO
		PS5::PS5Controller* GetPS5Controller() { return mPS5Controller; }

		void SetPS5Controller(PS5::PS5Controller* ps5Controller) { mPS5Controller = ps5Controller; }

	private:

		bool mLastFrameSelectPressed = false;

		bool mLastFrameCrouchPressed = false;

		bool mLastFrameInteractPressed = false;
		bool mLastFrameInteractHeld = false;

		bool mLastFrameDropPressed = false;

		bool mLastFrameItemUsePressed = false;

		PS5::PS5Controller* mPS5Controller;
#endif
	};
}

