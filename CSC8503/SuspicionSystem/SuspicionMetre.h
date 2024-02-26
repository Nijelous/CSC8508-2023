#pragma once
#include <map>

namespace SuspicionSystem
{
	class SuspicionMetre
	{
	public:
		const enum SusBreakpoint
		{
			low, mid, high
		};

		SusBreakpoint GetSusBreakpoint(const float &inSusMetre);
	private:
		const std::map<const float, const SusBreakpoint> mSusBreakpointMap = 
		{ { 66 , high} ,
		  { 33 , mid} ,
	      { 0  , low} };
	};
}
