#include "LocationBasedSuspicion.h"
#include <algorithm>

using namespace SuspicionSystem;

void LocationBasedSuspicion::Init(){
	mLocationSusAmountMap.clear();
	mActiveLocationSusCauseMap.clear();
}

void LocationBasedSuspicion::AddInstantLocalSusCause(instantLocationSusCause inCause, Vector3 pos){
	CantorPair pairedLocation(pos);
	CantorPair nearbyPairedLocation;

	if (!IsNearbySusLocation(pairedLocation, nearbyPairedLocation))
	{
		AddNewLocation(pairedLocation, mInstantLocationSusCauseSeverityMap[inCause]);
		AddActiveLocationSusCause(passiveRecovery, pairedLocation);
		return;
	}

	ChangeSusLocationSusAmount(nearbyPairedLocation, mInstantLocationSusCauseSeverityMap[inCause]);
}

bool LocationBasedSuspicion::AddActiveLocationSusCause(activeLocationSusCause inCause, Vector3 pos){
	CantorPair pairedLocation(pos);
	CantorPair nearbyPairedLocation;

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

bool LocationBasedSuspicion::RemoveActiveLocationSusCause(activeLocationSusCause inCause, Vector3 pos){
	CantorPair pairedLocation(pos);
	CantorPair nearbyPairedLocation;

	if (IsNearbySusLocation(pairedLocation, nearbyPairedLocation) &&
		IsActiveLocationsSusCause(inCause, nearbyPairedLocation))
	{
		mActiveLocationSusCauseMap[nearbyPairedLocation].clear();
		mActiveLocationSusCauseMap.erase(nearbyPairedLocation);
		return true;
	}

	return false;
}

bool LocationBasedSuspicion::AddActiveLocationSusCause(activeLocationSusCause inCause, CantorPair pairedLocation)
{
	Vector3 outPos = CantorPair::InverseCantorPair(pairedLocation);
	return AddActiveLocationSusCause(inCause, outPos);
}

bool LocationBasedSuspicion::RemoveActiveLocationSusCause(activeLocationSusCause inCause, CantorPair pairedLocation){
	Vector3 outPos = CantorPair::InverseCantorPair(pairedLocation);
	return RemoveActiveLocationSusCause(inCause, outPos);
}

void LocationBasedSuspicion::Update(float dt){
	if (mActiveLocationSusCauseMap.empty())
		return;
	for (auto entry = mActiveLocationSusCauseMap.begin(); entry != mActiveLocationSusCauseMap.end(); ++entry)
	{
		std::vector<activeLocationSusCause> vector = entry->second;
		auto pairedLocation = entry->first;

		for (int i = 0; i < vector.size(); i++)
		{
			ChangeSusLocationSusAmount(pairedLocation, activeLocationSusCauseSeverityMap[vector[i]] * dt);
		}
	}
}

int LocationBasedSuspicion::GetLocationSusAmount(Vector3 pos){
	CantorPair pairedLocation(pos);
	CantorPair nearbyPairedLocation;

	if (IsNearbySusLocation(pairedLocation, nearbyPairedLocation))
	{
		return mLocationSusAmountMap[nearbyPairedLocation];
	}

	return -1;
}

bool LocationBasedSuspicion::IsNearbySusLocation(CantorPair pairedLocation, CantorPair& nearbyPairedLocation){
	if (mLocationSusAmountMap.find(pairedLocation) != mLocationSusAmountMap.end())
	{
		nearbyPairedLocation = pairedLocation;
		return true;
	}

	Vector3 pos1 = CantorPair::InverseCantorPair(pairedLocation);
	Vector3 pos2;

	for (const auto& mapEntry : mLocationSusAmountMap) {

		pos2 = CantorPair::InverseCantorPair(mapEntry.first);

		if(CalculateDistance(pos1.x, pos2.x, pos1.y, pos2.y) <= MAX_NEARBY_DISTANCE)
		{
			nearbyPairedLocation = mapEntry.first;
			return true;
		}
	}

	return false;
}

void LocationBasedSuspicion::ChangeSusLocationSusAmount(CantorPair pairedLocation, float amount){
	mLocationSusAmountMap[pairedLocation] += amount;
	mLocationSusAmountMap[pairedLocation] = std::clamp
	(
		mLocationSusAmountMap[pairedLocation],
		0.0f,
		100.0f
	);

	if (mLocationSusAmountMap[pairedLocation] <= 0)
	{
		mActiveLocationSusCauseMap[pairedLocation].clear();
		mActiveLocationSusCauseMap.erase(pairedLocation);
		mLocationSusAmountMap.erase(pairedLocation);
	}

}

void LocationBasedSuspicion::AddNewLocation(CantorPair pairedLocation, float initSusAmount){
	if (initSusAmount <= 0)
		return;

	mLocationSusAmountMap[pairedLocation] = initSusAmount;
	mActiveLocationSusCauseMap[pairedLocation] = {passiveRecovery};
}

bool LocationBasedSuspicion::IsActiveLocationsSusCause(activeLocationSusCause inCause, CantorPair pairedLocation){
	auto foundCause = std::find(mActiveLocationSusCauseMap[pairedLocation].begin(),
		mActiveLocationSusCauseMap[pairedLocation].end(), inCause);

	//If the foundCause is not already int the activeSusCauses vector of that player
	if (foundCause != mActiveLocationSusCauseMap[pairedLocation].end())
	{
		return true;
	}

	return false;
}

float LocationBasedSuspicion::Calculate2DDistance(Vector3 inPos1, Vector3 inPos2) const{
	Vector2 outVector(inPos1.x - inPos2.x, inPos1.y - inPos2.y);
	return outVector.Length();
}



