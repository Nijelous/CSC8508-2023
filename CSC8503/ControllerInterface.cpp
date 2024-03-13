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
	if (mPS5Controller->GetNamedButton(("Cross")))
		return true;
#endif
	return false;
}

bool ControllerInterface::GetInteractPressed() {
	return false;
}

bool ControllerInterface::GetDropItemPressed() {
	return false;
}

bool ControllerInterface::GetSwitchItemPressed() {
	return false;
}

bool ControllerInterface::GetSprintDown() {
	return false;
}

bool ControllerInterface::GetCrouchPressed() {
	return false;
}
