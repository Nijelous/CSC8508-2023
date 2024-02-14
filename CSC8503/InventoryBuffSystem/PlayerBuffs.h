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
		Null, disguiseBuffApplied, disguiseBuffRemoved,slowApplied, slowRemoved, playerMakesSound
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
			Null, disguiseBuff, slow, makeSound, slowEveryoneElse, everyoneElseMakesSound
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
			slowEveryoneElse, everyoneElseMakesSound
		};

		std::map<buff, float> mBuffInitDurationMap =
		{
			{disguiseBuff,10}, {slowEveryoneElse,0}, {everyoneElseMakesSound,0}, {slow,4}
		};

		std::map<buff, BuffEvent> mOnBuffAppliedBuffEventMap =
		{
			{disguiseBuff, disguiseBuffApplied}, {slow,slowApplied}, {makeSound, playerMakesSound}
		};

		std::map < buff, BuffEvent> mOnBuffTickBuffEventMap =
		{

		};

		std::map < buff, BuffEvent> mOnBuffRemovedBuffEventMap =
		{
			{disguiseBuff, disguiseBuffRemoved}, {slow, slowRemoved}
		};

		std::map<buff, std::function<void(int playerNo)>> mOnBuffAppliedFunctionMap
		{
			{slowEveryoneElse,[this](int playerNo)
				{
					for (int i = 0; i < NCL::CSC8503::MAX_PLAYERS; i++){
						if(i!=playerNo){
							ApplyBuffToPlayer(slow,i);
						}
					}
				}
			},
			{everyoneElseMakesSound,[this](int playerNo)
				{
					for (int i = 0; i < NCL::CSC8503::MAX_PLAYERS; i++) {
						if (i != playerNo) {
							ApplyBuffToPlayer(makeSound,i);
						}
					}
				}
			}
		};

		std::map<buff, float> mActiveBuffDurationMap[NCL::CSC8503::MAX_PLAYERS];
		std::list<PlayerBuffsObserver*> mBuffsObserverList;
	};
}
