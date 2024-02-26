#pragma once
#include "SuspicionMetre.h"
#include <vector>


namespace SuspicionSystem
{
    class GlobalSuspicionObserver
    {
    public:
        virtual void UpdateGlobalSuspicionObserver(SuspicionMetre::SusBreakpoint susBreakpoint) = 0;
    };

    const int DT_UNTIL_GlOBAL_RECOVERY = 8;

    class GlobalSuspicionMetre :
        public SuspicionMetre
    {
    public:
        const enum instantGlobalSusCause
        {
            alarmTriggered, flagCaptured
        };

        const enum continuousGlobalSusCause
        {
            passiveRecovery
        };

        void Init();
        void AddInstantGlobalSusCause(const instantGlobalSusCause &inCause);

        void AddContinuousGlobalSusCause(const continuousGlobalSusCause &inCause);
        void RemoveContinuousGlobalSusCause(const continuousGlobalSusCause &inCause);

        float GetGlobalSusMeter()
        {
            return mGlobalSusMeter;
        };

        SuspicionMetre::SusBreakpoint GetGlobalSusMetreBreakpoint() {
            return SuspicionMetre::GetSusBreakpoint(mGlobalSusMeter);
        }

        void Attach(GlobalSuspicionObserver* observer);
        void Detach(GlobalSuspicionObserver* observer);
        void Notify(const SuspicionMetre::SusBreakpoint susBreakpoint);

        void Update(float dt);
    private:
        std::map<const instantGlobalSusCause, const float>  mInstantCauseSusSeverityMap =
        {
            {alarmTriggered,3}, {flagCaptured, 2}
        };

        std::map<const continuousGlobalSusCause, const float>  mContinuousCauseSusSeverityMap =
        {
            {passiveRecovery, -1}
        };

        float mGlobalSusMeter;
        float mGlobalRecoveryCooldown;
        std::vector<continuousGlobalSusCause> mContinuousGlobalSusCauseVector;
        std::list<GlobalSuspicionObserver*> mGlobalSuspicionObserverList;

        void ChangePlayerGlobalSusMetre(const float amount);
        void SetPlayerGlobalSusMetre(const float amount);
    };
}


