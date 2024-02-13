#include "PlayerBuffs.h"
#include "Level.h"

using namespace InventoryBuffSystem;
using namespace NCL::CSC8503;

void PlayerBuffs::Init()
{
	for (int playerNo = 0; playerNo < NCL::CSC8503::MAX_PLAYERS; playerNo++)
		mActiveBuffDurationMap[playerNo].clear();
}

void PlayerBuffs::ApplyBuffToPlayer(buff inBuff, int playerNo)
{
	Notify(mOnBuffAppliedBuffEventMap[inBuff], playerNo);
	mOnBuffAppliedFunctionMap[inBuff](playerNo);
	if (mBuffInitDurationMap[inBuff] != 0)
	{
		mActiveBuffDurationMap[playerNo][inBuff] = mBuffInitDurationMap[inBuff];
	}
}

void PlayerBuffs::RemoveBuffFromPlayer(buff inBuff, int playerNo)
{
	auto foundBuff = mActiveBuffDurationMap[playerNo].find(inBuff);

	if (foundBuff != mActiveBuffDurationMap[playerNo].end())
	{
		Notify(mOnBuffRemovedBuffEventMap[inBuff],playerNo);
		mActiveBuffDurationMap[playerNo].erase(foundBuff);
	}
};

PlayerBuffs::buff PlayerBuffs::GetRandomBuffFromPool(unsigned int seed)
{
	std::mt19937 rng(seed);
	std::shuffle(mBuffsInRandomPool.begin(), mBuffsInRandomPool.end(), rng);
	return mBuffsInRandomPool[0];
}

void PlayerBuffs::Update(float dt)
{
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
				RemoveBuffFromPlayer(entry->first, playerNo);
			}
		}
	}
}

void InventoryBuffSystem::PlayerBuffs::UpdateInventoryObserver(InventoryEvent invEvent, int playerNo)
{
	switch (invEvent)
	{
	case disguiseItemUsed:
		ApplyBuffToPlayer(PlayerBuffs::disguiseBuff, playerNo);
	default:
		break;
	}
}

void InventoryBuffSystem::PlayerBuffs::Attach(PlayerBuffsObserver* observer)
{
	mBuffsObserverList.push_back(observer);
}

void InventoryBuffSystem::PlayerBuffs::Detach(PlayerBuffsObserver* observer)
{
	mBuffsObserverList.remove(observer);
}

void InventoryBuffSystem::PlayerBuffs::Notify(BuffEvent buffEvent, int playerNo)
{
	std::list<PlayerBuffsObserver*>::iterator iterator = mBuffsObserverList.begin();
	while (iterator != mBuffsObserverList.end()) {
		(*iterator)->UpdatePlayerBuffsObserver(buffEvent, playerNo);
		++iterator;
	}
}
