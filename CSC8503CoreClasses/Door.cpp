#include "Door.h"

using namespace NCL::CSC8503;

void Door::Open()
{
	if (!this->IsActive())
		SetActive();
	mIsOpen = true;
}

void Door::Close()
{
	if (this->IsActive())
		SetActive();
	mIsOpen = false;
}
