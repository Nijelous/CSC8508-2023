#include "PrisonDoor.h"

using namespace NCL::CSC8503;

void PrisonDoor::UpdateGlobalSuspicionObserver(SuspicionSystem::SuspicionMetre::SusBreakpoint susBreakpoint){
	switch (susBreakpoint)
	{
	case SuspicionSystem::SuspicionMetre::high:
		Close();
		break;
	default:
		break;
	}
}
