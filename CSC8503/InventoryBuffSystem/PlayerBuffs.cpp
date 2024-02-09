#include "PlayerBuffs.h"

using namespace InventoryBuffSystem;

void PlayerBuffs::Init()
{
	for (int playerNo = 0; playerNo < MAX_PLAYERS; playerNo++)
		mActiveBuffDurationMap[playerNo].clear();

	mBuffsInRandomPool =
	{
		disguise, buff2
	};
}

void PlayerBuffs::ApplyBuffToPlayer(buff inBuff, int playerNo)
{
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
		mOnBuffRemovedFunctionMap[inBuff](playerNo);
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
	for (int playerNo = 0; playerNo < MAX_PLAYERS; playerNo++)
	{
		for (auto entry = mActiveBuffDurationMap[playerNo].begin();
			entry != mActiveBuffDurationMap[playerNo].end(); ++entry)
		{
			entry->second -= dt;

			if (entry->second > 0)
			{
				mOnBuffTickFunctionMap[entry->first](playerNo, dt);
			}
			else
			{
				RemoveBuffFromPlayer(entry->first, playerNo);
			}
		}
	}
}
