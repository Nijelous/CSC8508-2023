#include "Door.h"
#include "InteractableDoor.h"

using namespace NCL::CSC8503;

void InteractableDoor::Unlock(){
	mIsLocked = false;
}

void InteractableDoor::Lock(){
	mIsLocked = true;
}

void InteractableDoor::Interact()
{
	if (!CanBeInteractedWith())
		return;

	Open();
}

bool InteractableDoor::CanBeInteractedWith()
{
	return (!mIsOpen && !mIsLocked);
}
