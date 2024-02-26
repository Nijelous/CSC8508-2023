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
		Null, disguiseBuffApplied, disguiseBuffRemoved, slowApplied, slowRemoved,
		playerMakesSound, silentSprintApplied, silentSprintRemoved, speedApplied,
		speedRemoved, stunApplied, stunRemoved
	};

	class PlayerBuffsObserver
	{
	public:
		virtual void UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo) = 0;
	};
	class PlayerBuffs : public PlayerInventoryObserver
	{
	public:
		const enum buff
		{
			Null, disguiseBuff, slow, makeSound, slowEveryoneElse,
			everyoneElseMakesSound, silentSprint, speed,
			stun
		};
		PlayerBuffs() {
			Init();
		};

		~PlayerBuffs() {
			
		};
		void Init();
		void ApplyBuffToPlayer(const buff &inBuff, const int &playerNo);
		void RemoveBuffFromPlayer(const buff &inBuff, const int &playerNo);
		PlayerBuffs::buff GetRandomBuffFromPool(unsigned int seed);
		void Update(float dt);

		virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved = false) override;

		void Attach(PlayerBuffsObserver* observer);
		void Detach(PlayerBuffsObserver* observer);
		void Notify(BuffEvent buffEvent, int playerNo);

	private:
		std::vector< buff> mBuffsInRandomPool =
		{
			speed, silentSprint, slow,
		};

		std::map<const buff, const float> mBuffInitDurationMap =
		{
			{disguiseBuff,20}, {slow,8}, {silentSprint, 8}, {speed, 10}, {stun,3}
		};

		std::map<const buff, const BuffEvent> mOnBuffAppliedBuffEventMap =
		{
			{disguiseBuff, disguiseBuffApplied}, {slow,slowApplied},
			{makeSound, playerMakesSound}, {silentSprint, silentSprintApplied},
			{speed, speedApplied}, {stun, stunApplied}
		};

		std::map <const buff, const BuffEvent> mOnBuffTickBuffEventMap =
		{

		};

		std::map <const buff, const BuffEvent> mOnBuffRemovedBuffEventMap =
		{
			{disguiseBuff, disguiseBuffRemoved}, {slow, slowRemoved},
			{silentSprint, silentSprintRemoved}, {speed, speedRemoved},
			{stun, stunRemoved}
		};

		std::map<const buff, const std::function<void(int playerNo)>> mOnBuffAppliedFunctionMap
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
