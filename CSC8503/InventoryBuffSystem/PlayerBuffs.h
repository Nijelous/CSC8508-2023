#pragma once
#include <functional>
#include <random>
#include <map>
#include "Level.h"
#include "PlayerInventory.h"

using namespace NCL::CSC8503;

namespace InventoryBuffSystem {
	const enum BuffEvent
	{
		disguiseBuffApplied, disguiseBuffRemoved
	};

	class PlayerBuffsObserver
	{
	public:
		virtual void UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo) = 0;
	};
	class PlayerBuffs : public PlayerInventoryObserver
	{
	public:
		enum buff
		{
			disguiseBuff, buff2
		};

		void Init();
		void ApplyBuffToPlayer(buff inBuff, int playerNo);
		void RemoveBuffFromPlayer(buff inBuff, int playerNo);
		PlayerBuffs::buff GetRandomBuffFromPool(unsigned int seed);
		void Update(float dt);

		virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo) override;

		void Attach(PlayerBuffsObserver* observer);
		void Detach(PlayerBuffsObserver* observer);
		void Notify(BuffEvent buffEvent, int playerNo);

	private:
		std::vector<buff> mBuffsInRandomPool = 
		{
			disguiseBuff, buff2
		};

		std::map<buff, float> mBuffInitDurationMap =
		{
			{disguiseBuff,10},{buff2,4}
		};

		std::map<buff, BuffEvent> mOnBuffAppliedBuffEventMap =
		{
			{disguiseBuff, disguiseBuffApplied}
		};

		std::map < buff, BuffEvent> mOnBuffTickBuffEventMap =
		{

		};

		std::map < buff, BuffEvent> mOnBuffRemovedBuffEventMap =
		{
			{disguiseBuff, disguiseBuffRemoved}
		};

		std::map<buff, float> mActiveBuffDurationMap[NCL::CSC8503::MAX_PLAYERS];
		std::list<PlayerBuffsObserver*> mBuffsObserverList;
	};
}
