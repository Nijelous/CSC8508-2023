#pragma once
#include <functional>
#include <random>
#include <map>
#include "../SuspicionSystem/SuspicionSystem.h"

using namespace SuspicionSystem;

namespace InventoryBuffSystem {
		const int MAX_PLAYERS = 4;
	class PlayerBuffs
	{
	public:
		enum buff
		{
			disguise, slow, buff2, everyoneElseSlowsDown, everyoneElseMakesASound
		};

		void Init();
		void ApplyBuffToPlayer(buff inBuff, int playerNo);
		void RemoveBuffFromPlayer(buff inBuff, int playerNo);
		PlayerBuffs::buff GetRandomBuffFromPool(unsigned int seed);
		void Update(float dt);

	private:
		std::vector<buff> mBuffsInRandomPool = 
		{
			disguise, everyoneElseSlowsDown
		};

		std::map<buff, float> mBuffInitDurationMap =
		{
			{disguise,10},{buff2,4},{slow,4},{everyoneElseSlowsDown,0},{ever}
		};

		std::map<buff, std::function<void(int playerNo)>> mOnBuffAppliedFunctionMap =
		{
			{disguise, [](int playerNo)
				{
					SuspicionSystem::mLocalSuspicionMetre->AddActiveLocalSusCause(LocalSuspicionMetre::disguiseBuff,playerNo);
				}
			},
			{slow, [](int playerNo)
				{
					//Lower PlayerNo's movement speed
				}
			},
			{everyoneElseSlowsDown, [this](int playerNo)
				{
					for(int i=0;i<MAX_PLAYERS;i++)
						if (i != playerNo)
						{
							ApplyBuffToPlayer(slow, i);
						}
				}
			},
			{everyoneElseMakesASound, [](int playerNo)
				{
					for (int i = 0; i < MAX_PLAYERS; i++)
						if (i != playerNo)
						{
							mLocalSuspicionMetre->AddInstantLocalSusCause(LocalSuspicionMetre::soundEmitted,i);
							//Play sound at player i's location
						}
				}
			}
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
					SuspicionSystem::mLocalSuspicionMetre->RemoveActiveLocalSusCause(LocalSuspicionMetre::disguiseBuff,playerNo);
				}
			},
			{slow, [](int playerNo)
				{
					//Increase playerNo's movement speed to normal
				}
			},
		};

		std::map<buff, float> mActiveBuffDurationMap[MAX_PLAYERS];


	};

}
