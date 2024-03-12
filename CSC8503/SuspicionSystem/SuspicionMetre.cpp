#include "SuspicionMetre.h"

using namespace SuspicionSystem;

SuspicionMetre::SusBreakpoint SuspicionMetre::GetSusBreakpoint(const float& inSusMetre){
	auto mapIt = mSusBreakpointMap.begin();

	while (mapIt->first < inSusMetre)
		mapIt++;

	return mapIt->second;
}
