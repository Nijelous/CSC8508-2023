#pragma once
#include "LocalSuspicionMetre.h"
#include "GlobalSuspicionMetre.h"
#include "LocationBasedSuspicion.h"
#include "SuspicionMetre.h"

namespace SuspicionSystem 
{
	LocalSuspicionMetre* mLocalSuspicionMetre;
	GlobalSuspicionMetre* mGlobalSuspicionMetre;
	LocationBasedSuspicion* mLocationBasedSuspicion;
	void Init()
	{
		mGlobalSuspicionMetre = new GlobalSuspicionMetre();
		mLocalSuspicionMetre = new LocalSuspicionMetre(mGlobalSuspicionMetre);
		mLocationBasedSuspicion = new LocationBasedSuspicion();
	}

	void Reset()
	{
		mGlobalSuspicionMetre->Init();
		mLocalSuspicionMetre->Init();
		mLocationBasedSuspicion->Init();
	}
};