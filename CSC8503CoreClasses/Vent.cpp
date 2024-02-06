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
