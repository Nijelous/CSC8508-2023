#include "SuspicionMetre.h"

SuspicionMetre::SusBreakpoint SuspicionMetre::GetSusBreakpoint(float inSusMetre)
{
	std::map<float, SusBreakpoint>::iterator mapIt = mSusBreakpointMap.begin();

	while (mapIt->first > inSusMetre)
		mapIt++;

	return mapIt->second;
}
