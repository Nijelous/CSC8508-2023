#pragma once
#include "SuspicionMetre.h"
#include "GlobalSuspicionMetre.h"
#include <vector>
#include "../InventoryBuffSystem/PlayerBuffs.h"

using namespace NCL::CSC8503;
using namespace InventoryBuffSystem;

namespace SuspicionSystem
{
    const float DT_UNTIL_LOCAL_RECOVERY = 0.75f;
    class LocalSuspicionMetre :
        public SuspicionMetre, public PlayerBuffsObserver, public GlobalSuspicionObserver
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
            mGlobalSusMeterPTR->Attach(this);
            Init();
        }

        void Init();

        void AddInstantLocalSusCause(const instantLocalSusCause &inCause, const int &playerNo);

        bool IsActiveSusCauseForPlayer(const activeLocalSusCause& inCause, const int& playerNo);
        void AddActiveLocalSusCause(const activeLocalSusCause &inCause, const int &playerNo);
        void RemoveActiveLocalSusCause(const activeLocalSusCause &inCause, const int &playerNo);
        
        static double GetPercentageBasedOnDistance(const float distance,const float minVal = 30, const float minDist=10, const float maxDist = 50);

        virtual void UpdateGlobalSuspicionObserver(SuspicionMetre::SusBreakpoint susBreakpoint) override;
        virtual void UpdatePlayerBuffsObserver(BuffEvent buffEvent,int playerNo) override;

        float GetLocalSusMetreValue(const int &playerNo) {
            return mPlayerMeters[playerNo];
        }

        SuspicionMetre::SusBreakpoint GetLocalSusMetreBreakpoint(const int playerNo) {
            return SuspicionMetre::GetSusBreakpoint(mPlayerMeters[playerNo]);
        }

        void Update(float dt);
        void SyncActiveSusCauses(int playerID, int localPlayerID, activeLocalSusCause buffToSync, bool toApply);
        void SyncSusChange(int playerID, int localPlayerID, int changedValue);
    private:
        std::map<const instantLocalSusCause, const float>  mInstantLocalSusCauseSeverityMap =
        {
            {soundEmitted, 2}, {flagCapture, 40}
        };

        std::map<const activeLocalSusCause, const float>  mActiveLocalSusCauseSeverityMap =
        {
            {guardsLOS, 3}, {cameraLOS, 3}, {disguiseBuff, -20}, {playerWalk,3}, {playerSprint,9}, {passiveRecovery,-10}
        };

        float mPlayerMeters[NCL::CSC8503::MAX_PLAYERS];
        float mRecoveryCooldowns[NCL::CSC8503::MAX_PLAYERS];
        GlobalSuspicionMetre* mGlobalSusMeterPTR = nullptr;

        std::vector<activeLocalSusCause> mActiveLocalSusCauseVector[NCL::CSC8503::MAX_PLAYERS];
        std::vector<activeLocalSusCause> mActiveLocalSusCausesToRemove[NCL::CSC8503::MAX_PLAYERS];

        void ChangePlayerLocalSusMetre(const int &playerNo, const float &ammount);
        void HandleActiveSusCauseNetworking(const activeLocalSusCause& inCause, const int& playerNo, const bool& toApply);
        void HandleLocalSusChangeNetworking(const int& changedValue, const int& playerNo);
    };
}


