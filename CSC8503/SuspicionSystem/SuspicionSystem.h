#pragma once
#include "LocalSuspicionMetre.h"
#include "GlobalSuspicionMetre.h"
#include "LocationBasedSuspicion.h"
#include "SuspicionMetre.h"

namespace SuspicionSystem 
{
	class SuspicionSystemClass
	{
	public:
		void Init()
		{
			mGlobalSuspicionMetrePtr = new GlobalSuspicionMetre();
			mLocalSuspicionMetrePtr = new LocalSuspicionMetre(mGlobalSuspicionMetrePtr);
			mLocationBasedSuspicionPtr = new LocationBasedSuspicion();
		}

		void Reset()
		{
			mGlobalSuspicionMetrePtr->Init();
			mLocalSuspicionMetrePtr->Init();
			mLocationBasedSuspicionPtr->Init();
		}

		void Update(float dt)
		{
			mGlobalSuspicionMetrePtr->Update(dt);
			mLocalSuspicionMetrePtr->Update(dt);
			mLocationBasedSuspicionPtr->Update(dt);
		}

		LocalSuspicionMetre* GetLocalSuspicionMetre() { return mLocalSuspicionMetrePtr; };
		GlobalSuspicionMetre* GetGlobalSuspicionMetre() { return mGlobalSuspicionMetrePtr; };
		LocationBasedSuspicion* GetLocationBasedSuspicion() { return mLocationBasedSuspicionPtr; };
	private:
		LocalSuspicionMetre* mLocalSuspicionMetrePtr;
		GlobalSuspicionMetre* mGlobalSuspicionMetrePtr;
		LocationBasedSuspicion* mLocationBasedSuspicionPtr;
	};
};