#include "Vent.h"

using namespace NCL::CSC8503;

Vent::Vent() {
	mIsOpen = false;
	mConnectedVent = nullptr;
}

void Vent::ConnectVent(Vent* vent) {
	if (vent == nullptr) return;
	mConnectedVent = vent;
}

void Vent::Interact() {
	if (mIsOpen) {
		//TODO(erendgrmnc): teleport player to the other side of the vent.
	}
	else {
		//TODO(erendgrmnc): When levelManager is implemented, get local player reference and check is player can open vents.
		ToggleOpen();
	}

}
