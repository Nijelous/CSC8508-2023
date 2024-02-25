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

		if (activeLocationSusCauseSeverityMap[inCause] > 0)
			RemoveActiveLocationSusCause(passiveRecovery, nearbyPairedLocation);

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
		mActiveLocationSusCauseMap[nearbyPairedLocation].erase
		(std::remove(mActiveLocationSusCauseMap[nearbyPairedLocation].begin(),
			mActiveLocationSusCauseMap[nearbyPairedLocation].end(), inCause));

		if (mActiveLocationSusCauseMap[nearbyPairedLocation].empty() &&
			mLocationSusAmountMap[nearbyPairedLocation] >= 0)
			AddActiveLocationSusCause(passiveRecovery, nearbyPairedLocation);
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

	std::vector<CantorPair*> locationsToClear;

	for (auto entry = mActiveLocationSusCauseMap.begin(); entry != mActiveLocationSusCauseMap.end(); ++entry){
		std::vector<activeLocationSusCause> vector = entry->second;
		auto pairedLocation = entry->first;
		float tempSusAmount = 0;

		for (int i = 0; i < vector.size(); i++){
			tempSusAmount += activeLocationSusCauseSeverityMap[vector[i]];
		}

		ChangeSusLocationSusAmount(pairedLocation, tempSusAmount * dt);

		if (mLocationSusAmountMap[pairedLocation] <= 0 &&
			mActiveLocationSusCauseMap[pairedLocation].size() <= 1)
		{
			locationsToClear.push_back(&pairedLocation);
		}
	}

	for (CantorPair* thisLocation : locationsToClear)
	{
		mLocationSusAmountMap.erase(*thisLocation);
		mActiveLocationSusCauseMap[*thisLocation].clear();
		mActiveLocationSusCauseMap.erase(*thisLocation);
	}
	locationsToClear.clear();
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

bool LocationBasedSuspicion::IsNearbySusLocation(CantorPair pairedLocation, CantorPair& nearbyPairedLocation) const{
	nearbyPairedLocation = pairedLocation;
	if (mLocationSusAmountMap.find(pairedLocation) != mLocationSusAmountMap.end())
	{
		return true;
	}

	Vector3 pos1 = CantorPair::InverseCantorPair(pairedLocation);
	Vector3 pos2;

	for (const auto& mapEntry : mLocationSusAmountMap) {

		pos2 = CantorPair::InverseCantorPair(mapEntry.first);

		if(Calculate2DDistance(pos1, pos2) <= MAX_NEARBY_DISTANCE)
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

	float temp = mLocationSusAmountMap[pairedLocation];

}

void LocationBasedSuspicion::AddNewLocation(CantorPair pairedLocation, float initSusAmount){
	if (initSusAmount <= 0)
		return;

	mLocationSusAmountMap[pairedLocation] = initSusAmount;
}

bool LocationBasedSuspicion::IsActiveLocationsSusCause(activeLocationSusCause inCause, CantorPair pairedLocation) {
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
	Vector2 outVector(inPos1.x - inPos2.x, inPos1.z - inPos2.z);
	return outVector.Length();
}



