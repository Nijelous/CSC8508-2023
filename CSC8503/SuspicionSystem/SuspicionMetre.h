#pragma once
#include <map>

class SuspicionMetre
{
public:
	enum SusBreakpoint
	{
		low, mid, high
	};

	SusBreakpoint getSusBreakpoint(float inSusMetre);
private:
	std::map<float, SusBreakpoint> SusBreakpointMap = { { 66 , high} ,
														{ 33 , mid} ,
														{ 0 , low} };
};