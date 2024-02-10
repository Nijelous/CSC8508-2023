#include "LocationBasedSuspicion.h"
#include <algorithm>

using namespace SuspicionSystem;

void LocationBasedSuspicion::Init()
{
	mLocationSusAmountMap.clear();
	mLocationRecoveryCDMap.clear();
	mActiveLocationSusCauseMap.clear();
}

void LocationBasedSuspicion::AddInstantLocalSusCause(instantLocationSusCause inCause, float locationX, float locationZ)
{
	int pairedLocation = CantorPair(locationX,locationZ);
	int nearbyPairedLocation;

	if (!IsNearbySusLocation(pairedLocation, nearbyPairedLocation))
	{
		AddNewLocation(pairedLocation, mInstantLocationSusCauseSeverityMap[inCause]);
		return;
	}

	ChangeSusLocationSusAmount(nearbyPairedLocation, mInstantLocationSusCauseSeverityMap[inCause]);
}

bool LocationBasedSuspicion::AddActiveLocationSusCause(activeLocationSusCause inCause, float locationX, float locationZ)
{
	int pairedLocation = CantorPair(locationX, locationZ);
	int nearbyPairedLocation;

	if (!IsNearbySusLocation(pairedLocation, nearbyPairedLocation))
	{
		AddNewLocation(pairedLocation);
		mActiveLocationSusCauseMap[pairedLocation].push_back(inCause);

		return true;
	}

	if (!IsActiveLocationsSusCause(inCause, nearbyPairedLocation))
	{
		mActiveLocationSusCauseMap[nearbyPairedLocation].push_back(inCause);
		return true;
	}

	return true;
}

bool LocationBasedSuspicion::RemoveActiveLocationSusCause(activeLocationSusCause inCause, float locationX, float locationZ)
{
	int pairedLocation = CantorPair(locationX, locationZ);
	int nearbyPairedLocation;

	if (IsNearbySusLocation(pairedLocation, nearbyPairedLocation) &&
		IsActiveLocationsSusCause(inCause, nearbyPairedLocation))
	{
		mActiveLocationSusCauseMap[nearbyPairedLocation].clear();
		mActiveLocationSusCauseMap.erase(nearbyPairedLocation);
		return true;
	}

	return false;
}

bool LocationBasedSuspicion::RemoveActiveLocationSusCause(activeLocationSusCause inCause, int pairedLocation)
{
	int x, z;
	InverseCantorPair(pairedLocation, x, z);
	return RemoveActiveLocationSusCause(passiveRecovery, (float)x, float(z));
}

void LocationBasedSuspicion::Update(float dt)
{
	for (auto entry = mActiveLocationSusCauseMap.begin(); entry != mActiveLocationSusCauseMap.end(); ++entry)
	{
		std::vector<activeLocationSusCause> vector = entry->second;
		auto pairedLocation = entry->first;

		for (int i = 0; i < vector.size(); i++)
		{
			ChangeSusLocationSusAmount(pairedLocation, vector[i] * dt);
		}

		if (mLocationRecoveryCDMap[pairedLocation] == DT_UNTIL_LOCATION_RECOVERY)
		{
			RemoveActiveLocationSusCause(passiveRecovery, pairedLocation);
		}

		mLocationRecoveryCDMap[pairedLocation] -= dt;
		mLocationRecoveryCDMap[pairedLocation] = std::max(mLocationRecoveryCDMap[pairedLocation], 0.0f);

		if (mLocationRecoveryCDMap[pairedLocation] == 0)
		{
			RemoveActiveLocationSusCause(passiveRecovery, pairedLocation);
		}
	}
}

int LocationBasedSuspicion::GetLocationSusAmount(float locationX, float locationZ)
{
	int pairedLocation = CantorPair(locationX, locationZ);
	int nearbyPairedLocation;

	if (IsNearbySusLocation(pairedLocation, nearbyPairedLocation))
	{
		return mLocationSusAmountMap[nearbyPairedLocation];
	}

	return -1;
}

bool LocationBasedSuspicion::IsNearbySusLocation(int pairedLocation, int& nearbyPairedLocation)
{
	if (mLocationSusAmountMap.find(pairedLocation) != mLocationSusAmountMap.end())
	{
		nearbyPairedLocation = pairedLocation;
		return true;
	}

	int x1, x2, y1, y2;

	InverseCantorPair(pairedLocation,x1,y1);

	for (const auto& mapEntry : mLocationSusAmountMap) {

		InverseCantorPair(mapEntry.first, x2, y2);

		if(CalculateDistance(x1, x2, y1, y2) <= MAX_NEARBY_DISTANCE)
		{
			nearbyPairedLocation = mapEntry.first;
			return true;
		}
	}

	return false;
}

void LocationBasedSuspicion::ChangeSusLocationSusAmount(int pairedLocation, float amount)
{
	mLocationSusAmountMap[pairedLocation] += amount;
	mLocationSusAmountMap[pairedLocation] = std::clamp
	(
		mLocationSusAmountMap[pairedLocation],
		0.0f,
		100.0f
	);

	if (amount < 0)
		mLocationRecoveryCDMap[pairedLocation] = DT_UNTIL_LOCATION_RECOVERY;
}

void LocationBasedSuspicion::AddNewLocation(int pairedLocation, float initSusAmount)
{
	if (initSusAmount <= 0)
		return;

	mLocationSusAmountMap[pairedLocation] = initSusAmount;
	mLocationRecoveryCDMap[pairedLocation] = 0;
	mActiveLocationSusCauseMap[pairedLocation] = {};
}

bool LocationBasedSuspicion::IsActiveLocationsSusCause(activeLocationSusCause inCause, int pairedLocation)
{
	auto foundCause = std::find(mActiveLocationSusCauseMap[pairedLocation].begin(),
		mActiveLocationSusCauseMap[pairedLocation].end(), inCause);

	//If the foundCause is not already int the activeSusCauses vector of that player
	if (foundCause != mActiveLocationSusCauseMap[pairedLocation].end())
	{
		return true;
	}

	return false;
}

float LocationBasedSuspicion::CalculateDistance(int x1,int x2,int y1,int y2)
{
	return std::sqrt(std::pow(x2 - x1, 2)
		+ std::pow(y2 - y1, 2));
}

int LocationBasedSuspicion::CantorPair(int x, int y)
{
	return ((x + y) * (x + y + 1)) / 2 + y;
}

void LocationBasedSuspicion::InverseCantorPair(int z, int& x, int& y) {
	int w = static_cast<int>((std::sqrt(8 * z + 1) - 1) / 2);
	int t = (w * w + w) / 2;

	y = z - t;
	x = w - y;
}


