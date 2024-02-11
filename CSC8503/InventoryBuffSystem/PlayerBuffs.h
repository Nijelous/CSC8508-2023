#pragma once
#include <functional>
#include <random>
#include <map>
#include "Level.h"

using namespace NCL::CSC8503;

namespace InventoryBuffSystem {
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

	private:
		std::vector<buff> mBuffsInRandomPool = 
		{
			disguise, buff2
		};

		std::map<buff, float> mBuffInitDurationMap =
		{
			{disguise,10},{buff2,4}
		};

		std::map<buff, std::function<void(int playerNo)>> mOnBuffAppliedFunctionMap =
		{
			{disguise, [](int playerNo)
				{

				}
			},
			{buff2, [](int playerNo)
				{

				}
			},
		};

		std::map < buff, std::function<void(int playerNo,float dt)>> mOnBuffTickFunctionMap =
		{
			{disguise, [](int playerNo,float dt)
				{

				}
			},
			{buff2, [](int playerNo,float dt)
				{

				}
			},
		};

		std::map < buff, std::function<void(int playerNo)>> mOnBuffRemovedFunctionMap =
		{
			{disguise, [](int playerNo)
				{

				}
			},
			{buff2, [](int playerNo)
				{

				}
			},
		};

		std::map<buff, float> mActiveBuffDurationMap[NCL::CSC8503::MAX_PLAYERS];


	};

}
