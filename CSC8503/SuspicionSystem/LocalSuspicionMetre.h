#pragma once
#include "SuspicionMetre.h"
#include "GlobalSuspicionMetre.h"
#include <vector>

namespace SuspicionSystem{
    const int MAX_PLAYERS = 4;
    const float DT_UNTIL_LOCAL_RECOVERY = 5;

    class LocalSuspicionMetre :
        public SuspicionMetre
    {
    public:
        const enum instantLocalSusCause
        {
            soundEmitted, flagCapture
        };

        const enum activeLocalSusCause
        {
            guardsLOS, cameraLOS, hiddenInShadow, passiveRecovery
        };

        LocalSuspicionMetre(GlobalSuspicionMetre* globalSusMeterPTR) {
            this->mGlobalSusMeterPTR = globalSusMeterPTR;
            Init();
        }
    
        void Init();

        void AddInstantLocalSusCause(const instantLocalSusCause inCause, int playerNo);

        bool AddActiveLocalSusCause(activeLocalSusCause inCause, int playerNo);
        bool RemoveActiveLocalSusCause(activeLocalSusCause inCause, int playerNo);

        float GetLocalSusMetreValue(int playerNo) {
            return mPlayerMeters[playerNo];
        }

        SuspicionMetre::SusBreakpoint GetLocalSusMetreBreakpoint(int playerNo) {
            return SuspicionMetre::getSusBreakpoint(mPlayerMeters[playerNo]);
        }

        void Update(float dt);

    private:
        std::map<instantLocalSusCause, float>  instantLocalSusCauseSeverityMap =
        {
            {soundEmitted, 2}, {flagCapture, 40}
        };

        std::map<activeLocalSusCause, float>  activeLocalSusCauseSeverityMap =
        {
            {guardsLOS, 3}, {cameraLOS, 3}
        };

        float mPlayerMeters[MAX_PLAYERS];
        float mRecoveryCooldowns[MAX_PLAYERS];
        GlobalSuspicionMetre* mGlobalSusMeterPTR = nullptr;

        std::vector<activeLocalSusCause> mActiveLocalSusCauseVector[MAX_PLAYERS];

        void ChangePlayerLocalSusMetre(int playerNo, float amount);
    };
}
