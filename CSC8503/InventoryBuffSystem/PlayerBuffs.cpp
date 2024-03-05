#include "PlayerBuffs.h"
#include "Level.h"
#include "GameServer.h"
#include "NetworkObject.h"
#include "../DebugNetworkedGame.h"
#include "../SceneManager.h"
#include "../CSC8503/LevelManager.h"

using namespace InventoryBuffSystem;
using namespace NCL::CSC8503;

void PlayerBuffs::Init(){
	for (int playerNo = 0; playerNo < NCL::CSC8503::MAX_PLAYERS; playerNo++)
		mActiveBuffDurationMap[playerNo].clear();

	mBuffsObserverList.clear();
	mBuffsToRemove->clear();
}

void PlayerBuffs::ApplyBuffToPlayer(const buff& inBuff, const int& playerNo){
	//mOnBuffAppliedFunctionMap[inBuff](playerNo);
	auto foundBuffDuration = mBuffInitDurationMap.find(inBuff);
	if (foundBuffDuration != mBuffInitDurationMap.end())
	{
		mActiveBuffDurationMap[playerNo][inBuff] = mBuffInitDurationMap[inBuff];
	}

	HandleBuffNetworking(inBuff, playerNo,true);
	Notify(mOnBuffAppliedBuffEventMap[inBuff], playerNo);
}

void PlayerBuffs::RemoveBuffFromPlayer(const buff& inBuff, const int& playerNo){
	auto foundBuff = mActiveBuffDurationMap[playerNo].find(inBuff);

	if (foundBuff != mActiveBuffDurationMap[playerNo].end())
	{
		mBuffsToRemove[playerNo].push_back(inBuff);

		HandleBuffNetworking(inBuff, playerNo, false);
		Notify(mOnBuffRemovedBuffEventMap[inBuff], playerNo);
	}
};

void PlayerBuffs::HandleBuffNetworking(const buff& inBuff, const int& playerNo, const bool& toApply) {
	int localPlayerId = 0;
	DebugNetworkedGame* game = reinterpret_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
	if (!SceneManager::GetSceneManager()->IsInSingleplayer()) {
		const auto* localPlayer = game->GetLocalPlayer();
		localPlayerId = localPlayer->GetPlayerID();

		const bool isServer = game->GetIsServer();
		if (isServer) {
			game->SendClientSyncBuffPacket(playerNo, inBuff, toApply);
		}
	}
}

PlayerBuffs::buff PlayerBuffs::GetRandomBuffFromPool(unsigned int seed, std::vector<buff>* randomBuffPool){
	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle((*randomBuffPool).begin(), (*randomBuffPool).end(), gen);
	return (*randomBuffPool)[0];
}

void PlayerBuffs::Update(float dt){
	for (int playerNo = 0; playerNo < NCL::CSC8503::MAX_PLAYERS; playerNo++)
	{
		for (auto entry = mActiveBuffDurationMap[playerNo].begin();
			entry != mActiveBuffDurationMap[playerNo].end(); ++entry)
		{
			entry->second -= dt;

			auto foundEvent = mOnBuffTickBuffEventMap.find(entry->first);

			if (entry->second > 0)
			{
				if (foundEvent != mOnBuffTickBuffEventMap.end())
					Notify(mOnBuffTickBuffEventMap[entry->first],playerNo);
			}
			else
			{
				RemoveBuffFromPlayer(entry->first, playerNo);
			}
		}

		for (const buff thisBuff : mBuffsToRemove[playerNo])
		{
			mActiveBuffDurationMap[playerNo].erase(thisBuff);
		}

		mBuffsToRemove[playerNo].clear();
	}
}

void InventoryBuffSystem::PlayerBuffs::UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemove){
	switch (invEvent) {
	case disguiseItemUsed:
		ApplyBuffToPlayer(PlayerBuffs::disguiseBuff, playerNo);
	default:
		break;
	}
}

void InventoryBuffSystem::PlayerBuffs::Attach(PlayerBuffsObserver* observer){
	mBuffsObserverList.push_back(observer);
}

void InventoryBuffSystem::PlayerBuffs::Detach(PlayerBuffsObserver* observer){
	mBuffsObserverList.remove(observer);
}

void InventoryBuffSystem::PlayerBuffs::Notify(BuffEvent buffEvent, int playerNo){

	std::list<PlayerBuffsObserver*>::iterator iterator = mBuffsObserverList.begin();
	
	while (iterator != mBuffsObserverList.end()) {
		(*iterator)->UpdatePlayerBuffsObserver(buffEvent, playerNo);
		++iterator;
	}
}

float PlayerBuffs::GetBuffDuration(PlayerBuffs::buff inBuff) {
	return mBuffInitDurationMap[inBuff];
}

void PlayerBuffs::SyncPlayerBuffs(int playerID, int localPlayerID, buff buffToSync, bool toApply){
	if (localPlayerID != playerID)
		return;

	if (toApply)
		ApplyBuffToPlayer(buffToSync, localPlayerID);
	else
		RemoveBuffFromPlayer(buffToSync, localPlayerID);
}