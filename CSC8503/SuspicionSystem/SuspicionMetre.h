#pragma once
#include <map>
namespace SuspicionSystem
{
	class SuspicionMetre
	{
	public:
		enum SusBreakpoint
		{
			low, mid, high
		};

		SusBreakpoint GetSusBreakpoint(float inSusMetre);
	private:
		std::map<float, SusBreakpoint> mSusBreakpointMap = { { 66 , high} ,
															{ 33 , mid} ,
															{ 0 , low} };
	};
}