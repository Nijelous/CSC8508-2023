#include "LocationBasedSuspicion.h"
#include <algorithm>
#include <limits>
#include "GameClient.h"
#include "../DebugNetworkedGame.h"
#include "../SceneManager.h"

using namespace SuspicionSystem;
using namespace NCL::CSC8503;

void LocationBasedSuspicion::Init(){
	mLocationSusAmountMap.clear();
	mActiveLocationSusCauseMap.clear();
	mVec3LocationSusAmountMap.clear();
	mActiveLocationlSusCausesToRemove.clear();
	locationsToClear.clear();
}

void LocationBasedSuspicion::AddInstantLocalSusCause(instantLocationSusCause inCause, Vector3 pos){
	CantorPair pairedLocation(pos);
	CantorPair nearbyPairedLocation;

	if (!IsNearbySusLocation(pairedLocation, nearbyPairedLocation))
	{
		AddNewLocation(pairedLocation, mInstantLocationSusCauseSeverityMap[inCause]);
		UpdateVec3LocationSusAmountMap();
		return;
	}

	ChangeSusLocationSusAmount(nearbyPairedLocation, mInstantLocationSusCauseSeverityMap[inCause]);
#ifdef USEGL
	HandleSusChangeNetworking(mLocationSusAmountMap[nearbyPairedLocation], nearbyPairedLocation);
#endif
}

bool LocationBasedSuspicion::AddActiveLocationSusCause(activeLocationSusCause inCause, Vector3 pos){
	CantorPair pairedLocation(pos);
	CantorPair nearbyPairedLocation;
	if (!IsNearbySusLocation(pairedLocation, nearbyPairedLocation))
	{
		AddNewLocation(pairedLocation);
		mActiveLocationSusCauseMap[pairedLocation].push_back(inCause);
#ifdef USEGL
		HandleActiveSusCauseNetworking(inCause, nearbyPairedLocation, true);
#endif
		return true;
	}

	if (!IsActiveLocationsSusCause(inCause, nearbyPairedLocation))
	{
		mActiveLocationSusCauseMap[nearbyPairedLocation].push_back(inCause);
#ifdef USEGL
		HandleActiveSusCauseNetworking(inCause, nearbyPairedLocation, true);
#endif
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
		mActiveLocationlSusCausesToRemove[nearbyPairedLocation].push_back(inCause);
#ifdef USEGL
		HandleActiveSusCauseNetworking(inCause,nearbyPairedLocation,false);
#endif
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
	if (mActiveLocationSusCauseMap.empty() && mLocationSusAmountMap.empty())
		return;

	for (auto entry = mActiveLocationSusCauseMap.begin(); entry != mActiveLocationSusCauseMap.end(); ++entry){
		std::vector<activeLocationSusCause> vector = entry->second;
		auto pairedLocation = entry->first;
		float tempSusAmount = 0;

		for (int i = 0; i < vector.size(); i++){
			tempSusAmount += activeLocationSusCauseSeverityMap[vector[i]];
		}

		if (tempSusAmount != 0){
			ChangeSusLocationSusAmount(pairedLocation, tempSusAmount * dt);
#ifdef USEGL
			HandleSusChangeNetworking(mLocationSusAmountMap[pairedLocation], pairedLocation);
#endif
		}

		std::vector<activeLocationSusCause> susCausesToRemoveVector = mActiveLocationlSusCausesToRemove[pairedLocation];
		
		for (int i = 0; i < susCausesToRemoveVector.size(); i++){
			mActiveLocationSusCauseMap[pairedLocation].erase
			(std::remove(mActiveLocationSusCauseMap[pairedLocation].begin(),
				mActiveLocationSusCauseMap[pairedLocation].end(), susCausesToRemoveVector[i]));
		}
	}

	for (auto entry = mLocationSusAmountMap.begin(); entry != mLocationSusAmountMap.end(); ++entry) {
		auto pairedLocation = entry->first;
		ChangeSusLocationSusAmount(entry->first, activeLocationSusCauseSeverityMap[passiveRecovery] * dt);

		if (mLocationSusAmountMap[pairedLocation] <= 0 ) {
			locationsToClear.push_back(pairedLocation);
		}
	}

	for (CantorPair thisLocation : locationsToClear){
		mLocationSusAmountMap.erase(*thisLocation);
		mActiveLocationSusCauseMap[*thisLocation].clear();
		mActiveLocationSusCauseMap.erase(*thisLocation);
	}

	locationsToClear.clear();
	mActiveLocationlSusCausesToRemove.clear();
	UpdateVec3LocationSusAmountMap();
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

void SuspicionSystem::LocationBasedSuspicion::SetMinLocationSusAmount(Vector3 pos, float susValue)
{
	CantorPair pairedLocation(pos);
	CantorPair nearbyPairedLocation;

	if (!IsNearbySusLocation(pairedLocation, nearbyPairedLocation))
	{
		AddNewLocation(pairedLocation, susValue);
		return;
	}
		
	mLocationSusAmountMap[nearbyPairedLocation] = std::min(susValue, mLocationSusAmountMap[nearbyPairedLocation]);
#ifdef USEGL
	HandleSusChangeNetworking(mLocationSusAmountMap[nearbyPairedLocation], nearbyPairedLocation);
#endif
	UpdateVec3LocationSusAmountMap();
}

void LocationBasedSuspicion::UpdateVec3LocationSusAmountMap(){
	mVec3LocationSusAmountMap.clear();
	for (auto it = mLocationSusAmountMap.begin(); it != mLocationSusAmountMap.end(); ++it)
	{
		mVec3LocationSusAmountMap[CantorPair::InverseCantorPair(it->first)] = it->second;
	}
}

void LocationBasedSuspicion::SyncActiveSusCauses(const activeLocationSusCause& inCause, const int& pairedLocation, const bool& toApply){
	CantorPair cantorPairedLocation = CantorPair(pairedLocation);
	if (toApply)
		AddActiveLocationSusCause(inCause,cantorPairedLocation);
	else
		RemoveActiveLocationSusCause(inCause, cantorPairedLocation);
}

void LocationBasedSuspicion::SyncSusChange(const int& pairedLocation, const int& changedValue){
	CantorPair cantorPairedLocation = CantorPair(pairedLocation);
	CantorPair nearbyPairedLocation;

	if (!IsNearbySusLocation(pairedLocation, nearbyPairedLocation))
		return;
	mLocationSusAmountMap[nearbyPairedLocation] = changedValue;
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
#ifdef USEGL
	HandleSusChangeNetworking(initSusAmount, pairedLocation);
#endif
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

#ifdef USEGL
void LocationBasedSuspicion::HandleActiveSusCauseNetworking(const activeLocationSusCause& inCause, const CantorPair& pairedLocation, const bool& toApply){
	int localPlayerId = 0;
	DebugNetworkedGame* game = reinterpret_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
	if (!SceneManager::GetSceneManager()->IsInSingleplayer()) {
		const bool isServer = game->GetIsServer();
		if (isServer) {
			game->SendClientSyncLocationActiveSusCausePacket(pairedLocation, inCause, toApply);
		}
	}
}

void LocationBasedSuspicion::HandleSusChangeNetworking(const int& changedValue, const CantorPair& pairedLocation){
	int localPlayerId = 0;
	DebugNetworkedGame* game = reinterpret_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
	if (!SceneManager::GetSceneManager()->IsInSingleplayer()) {
		const bool isServer = game->GetIsServer();
		if (isServer) {
			game->SendClientSyncLocationSusChangePacket(pairedLocation, changedValue);
		}
		else{
			game->GetClient()->WriteAndSendSyncLocationSusChangePacket(pairedLocation, changedValue);
		}
	}
}
#endif

void LocationBasedSuspicion::RemoveSusLocation(const Vector3 pos){
	CantorPair pairedLocation(pos);
	CantorPair nearbyPairedLocation;

	auto it = std::find(locationsToClear.begin(),
		locationsToClear.end(), pairedLocation);

	if (it == locationsToClear.end()&&
		!IsNearbySusLocation(pairedLocation, nearbyPairedLocation))
		return;

	locationsToClear.push_back(nearbyPairedLocation);
}

