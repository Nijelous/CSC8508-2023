#include "SuspicionMetre.h"

SuspicionMetre::SusBreakpoint SuspicionMetre::getSusBreakpoint(float inSusMetre)
{
	std::map<float, SusBreakpoint>::iterator mapIt = SusBreakpointMap.begin();

	while (mapIt->first > inSusMetre)
		mapIt++;

	return mapIt->second;
}
