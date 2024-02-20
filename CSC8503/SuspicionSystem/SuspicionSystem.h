#pragma once
#include "LocalSuspicionMetre.h"
#include "GlobalSuspicionMetre.h"
#include "LocationBasedSuspicion.h"
#include "SuspicionMetre.h"
#include "../InventoryBuffSystem/InventoryBuffSystem.h"

using namespace InventoryBuffSystem;

namespace SuspicionSystem 
{
	class SuspicionSystemClass
	{
	public:
		SuspicionSystemClass(InventoryBuffSystemClass* InventoryBuffSystemClassPtr)
		{
			Init(InventoryBuffSystemClassPtr);
		}

		void Init(InventoryBuffSystemClass* InventoryBuffSystemClassPtr)
		{
			mGlobalSuspicionMetrePtr = new GlobalSuspicionMetre();
			mLocalSuspicionMetrePtr = new LocalSuspicionMetre(mGlobalSuspicionMetrePtr);
			InventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->Attach(mLocalSuspicionMetrePtr);
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
		InventoryBuffSystemClass* mInventoryBuffSystemClassPtr;
	};
};