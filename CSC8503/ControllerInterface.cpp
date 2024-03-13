#include "ControllerInterface.h"

#include "Keyboard.h"
#include "Window.h"

#ifdef USEPROSPERO
#include "../PS5Core/PS5Window.h"
#endif

using namespace  NCL;

bool NCL::ControllerInterface::MoveForward() {
#ifdef USEGL
	if (Window::GetKeyboard()->KeyDown(KeyCodes::W))
		return true;
#endif
#ifdef USEPROSPERO
	if (mPS5Controller->GetNamedAxis("Forward"))
		return true;
#endif
	return false;
}

bool ControllerInterface::MoveBackwards() {
#ifdef USEGL
	if (Window::GetKeyboard()->KeyDown(KeyCodes::S))
		return true;
#endif
#ifdef USEPROSPERO
	if (mPS5Controller->GetNamedAxis("Backwards"))
		return true;
#endif
	return false;
}

bool ControllerInterface::MoveRight() {
#ifdef USEGL
	if (Window::GetKeyboard()->KeyDown(KeyCodes::D))
		return true;
#endif
#ifdef USEPROSPERO
	if (mPS5Controller->GetNamedAxis("Sidestep"))
		return true;
#endif
	return false;
}

bool ControllerInterface::MoveLeft() {
#ifdef USEGL
	if (Window::GetKeyboard()->KeyDown(KeyCodes::A))
		return true;
#endif
#ifdef USEPROSPERO
	if (mPS5Controller->GetNamedAxis("Left"))
		return true;
#endif
	return false;
}

bool ControllerInterface::GetSelectPressed() {
#ifdef USEGL
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE))
		return true;
#endif
#ifdef USEPROSPERO
	if (mPS5Controller->GetNamedButton(("Cross"))) {
		if (!mLastFrameSelectPressed) {
			mLastFrameSelectPressed = true;
			return true;
		}
	}
	else {
		mLastFrameSelectPressed = false;
	}
#endif
	return false;
}

bool ControllerInterface::GetInteractPressed() {
#ifdef USEGL
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::E))
		return true;
#endif
#ifdef USEPROSPERO
	if (mPS5Controller->GetNamedButton(("Square"))) {
		if (!mLastFrameInteractPressed) {
			mLastFrameInteractPressed = true;
			return true;
		}
	}
	else {
		mLastFrameInteractPressed = false;
	}
#endif
	return false;
}

bool ControllerInterface::GetInteractHeld() {
#ifdef USEGL
	if (Window::GetKeyboard()->KeyHeld(KeyCodes::E))
		return true;
#endif
#ifdef USEPROSPERO
	if (mPS5Controller->GetNamedButton(("Square"))) {
		if (mLastFrameInteractHeld) {
			return true;
		}
		mLastFrameInteractHeld = true;
	}
	else {
		mLastFrameInteractHeld = false;
	}
#endif
	return false;
}

bool ControllerInterface::GetDropItemPressed() {
#ifdef USEGL
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q))
		return true;
#endif
#ifdef USEPROSPERO
	if (mPS5Controller->GetNamedButton(("Triangle"))) {
		if (!mLastFrameDropPressed) {
			mLastFrameDropPressed = true;
			return true;
		}
	}
	else {
		mLastFrameDropPressed = false;
	}
#endif
	return false;
}

bool ControllerInterface::CheckSwitchItemOne() {
#ifdef USEGL
	if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM1))
		return true;
#endif
#ifdef USEPROSPERO
	if (mPS5Controller->GetNamedButton(("L1")))
		return true;
#endif
	return false;
}

bool ControllerInterface::CheckSwitchItemTwo() {
#ifdef USEGL
	if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM2))
		return true;
#endif
#ifdef USEPROSPERO
	if (mPS5Controller->GetNamedButton(("R1")))
		return true;
#endif
	return false;
}

bool ControllerInterface::GetSprintDown() {
#ifdef USEGL
	if (Window::GetKeyboard()->KeyDown(KeyCodes::SHIFT))
		return true;
#endif
#ifdef USEPROSPERO
	if (mPS5Controller->GetNamedButton(("Circle")) || mPS5Controller->GetNamedButton(("L3")))
		return true;
#endif
	return false;
}

bool ControllerInterface::GetCrouchPressed() {
#ifdef USEGL
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::CONTROL))
		return true;
#endif
#ifdef USEPROSPERO
	if (mPS5Controller->GetNamedButton(("R3"))) {
		if (!mLastFrameCrouchPressed) {
			mLastFrameCrouchPressed = true;
			return true;
		}
	}
	else {
		mLastFrameCrouchPressed = false;
	}
#endif
	return false;
}

bool ControllerInterface::UseItemPressed() {
#ifdef USEGL
	if (Window::GetMouse()->ButtonPressed(MouseButtons::Left))
		return true;
#endif
#ifdef USEPROSPERO
	if (mPS5Controller->GetNamedButton(("R2"))) {
		if (!mLastFrameItemUsePressed) {
			mLastFrameItemUsePressed = true;
			return true;
		}
	}
	else {
		mLastFrameItemUsePressed = false;
	}
#endif
	return false;
}
