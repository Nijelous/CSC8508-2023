#pragma once
#include <functional>
#include <random>
#include <map>
#include "Level.h"

using namespace NCL::CSC8503;

namespace InventoryBuffSystem {
	const enum BuffEvent
	{
		
	};

	class PlayerBuffsObserver
	{
	public:
		virtual void UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo) = 0;
	};
	class PlayerBuffs
	{
	public:
		enum buff
		{
			disguise, buff2
		};

		void Init();
		void ApplyBuffToPlayer(buff inBuff, int playerNo);
		void RemoveBuffFromPlayer(buff inBuff, int playerNo);
		PlayerBuffs::buff GetRandomBuffFromPool(unsigned int seed);
		void Update(float dt);

		void Attach(PlayerBuffsObserver* observer);
		void Detach(PlayerBuffsObserver* observer);
		void Notify(BuffEvent buffEvent, int playerNo);

	private:
		std::vector<buff> mBuffsInRandomPool = 
		{
			disguise, buff2
		};

		std::map<buff, float> mBuffInitDurationMap =
		{
			{disguise,10},{buff2,4}
		};

		std::map<buff, BuffEvent> mOnBuffAppliedBuffEventMap =
		{

		};

		std::map < buff, BuffEvent> mOnBuffTickBuffEventMap =
		{

		};

		std::map < buff, BuffEvent> mOnBuffRemovedBuffEventMap =
		{

		};

		std::map<buff, float> mActiveBuffDurationMap[NCL::CSC8503::MAX_PLAYERS];
		std::list<PlayerBuffsObserver*> mBuffsObserverList;
	};

}
