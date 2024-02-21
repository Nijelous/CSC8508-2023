#pragma once
#include "SuspicionMetre.h"
#include "GlobalSuspicionMetre.h"
#include <vector>
#include "Level.h"
#include "../InventoryBuffSystem/PlayerBuffs.h"

using namespace NCL::CSC8503;
using namespace InventoryBuffSystem;

namespace SuspicionSystem
{
    const float DT_UNTIL_LOCAL_RECOVERY = 0.75f;
    class LocalSuspicionMetre :
        public SuspicionMetre, public PlayerBuffsObserver
    {
    public:
        const enum instantLocalSusCause
        {
            soundEmitted, flagCapture
        };

        const enum activeLocalSusCause
        {
            guardsLOS, cameraLOS, hiddenInShadow, disguiseBuff, passiveRecovery, playerWalk, playerSprint
        };

        LocalSuspicionMetre(GlobalSuspicionMetre* globalSusMeterPTR) {
            this->mGlobalSusMeterPTR = globalSusMeterPTR;
            Init();
        }

        void Init();

        void AddInstantLocalSusCause(const instantLocalSusCause inCause, int playerNo);

        bool AddActiveLocalSusCause(activeLocalSusCause inCause, int playerNo);
        bool RemoveActiveLocalSusCause(activeLocalSusCause inCause, int playerNo);

        virtual void UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo) override;

        float GetLocalSusMetreValue(int playerNo) {
            return mPlayerMeters[playerNo];
        }

        SuspicionMetre::SusBreakpoint GetLocalSusMetreBreakpoint(int playerNo) {
            return SuspicionMetre::GetSusBreakpoint(mPlayerMeters[playerNo]);
        }

        void Update(float dt);

    private:
        std::map<instantLocalSusCause, float>  mInstantLocalSusCauseSeverityMap =
        {
            {soundEmitted, 2}, {flagCapture, 40}
        };

        std::map<activeLocalSusCause, float>  mActiveLocalSusCauseSeverityMap =
        {
            {guardsLOS, 3}, {cameraLOS, 3}, {disguiseBuff, -6}, {playerWalk,3}, {playerSprint,9}, {passiveRecovery,-10}
        };

        float mPlayerMeters[NCL::CSC8503::MAX_PLAYERS];
        float mRecoveryCooldowns[NCL::CSC8503::MAX_PLAYERS];
        GlobalSuspicionMetre* mGlobalSusMeterPTR = nullptr;

        std::vector<activeLocalSusCause> mActiveLocalSusCauseVector[NCL::CSC8503::MAX_PLAYERS];

        void ChangePlayerLocalSusMetre(int playerNo, float ammount);
    };
}


