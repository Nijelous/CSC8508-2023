#include "PlayerBuffs.h"
#include "Level.h"

using namespace InventoryBuffSystem;
using namespace NCL::CSC8503;

void PlayerBuffs::Init(){
	for (int playerNo = 0; playerNo < NCL::CSC8503::MAX_PLAYERS; playerNo++)
		mActiveBuffDurationMap[playerNo].clear();

	mBuffsObserverList.clear();
}

void PlayerBuffs::ApplyBuffToPlayer(buff inBuff, int playerNo){
	Notify(mOnBuffAppliedBuffEventMap[inBuff], playerNo);
	//mOnBuffAppliedFunctionMap[inBuff](playerNo);
	auto foundBuffDuration = mBuffInitDurationMap.find(inBuff);
	if (foundBuffDuration != mBuffInitDurationMap.end())
	{
		mActiveBuffDurationMap[playerNo][inBuff] = mBuffInitDurationMap[inBuff];
	}
}

void PlayerBuffs::RemoveBuffFromPlayer(buff inBuff, int playerNo){
	auto foundBuff = mActiveBuffDurationMap[playerNo].find(inBuff);

	if (foundBuff != mActiveBuffDurationMap[playerNo].end())
	{
		Notify(mOnBuffRemovedBuffEventMap[inBuff],playerNo);
		mActiveBuffDurationMap[playerNo].erase(foundBuff);
	}
};

PlayerBuffs::buff PlayerBuffs::GetRandomBuffFromPool(unsigned int seed){
	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(mBuffsInRandomPool.begin(), mBuffsInRandomPool.end(), gen);
	return mBuffsInRandomPool[0];
}

void PlayerBuffs::Update(float dt){
	vector<buff> buffsToRemove;
	for (int playerNo = 0; playerNo < NCL::CSC8503::MAX_PLAYERS; playerNo++)
	{
		for (auto entry = mActiveBuffDurationMap[playerNo].begin();
			entry != mActiveBuffDurationMap[playerNo].end(); ++entry)
		{
			entry->second -= dt;

			if (entry->second > 0)
			{
				Notify(mOnBuffTickBuffEventMap[entry->first],playerNo);
			}
			else
			{
				buffsToRemove.push_back(entry->first);
			}
		}

		for (const buff thisBuff : buffsToRemove)
		{
			RemoveBuffFromPlayer(thisBuff, playerNo);
		}

		buffsToRemove.clear();
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
