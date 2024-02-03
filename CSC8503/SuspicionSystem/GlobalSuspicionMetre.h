#pragma once
#include "SuspicionMetre.h"
#include <vector>

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

    void AddInstantGlobalSusCause(instantGlobalSusCause inCause);

    void AddContinuousGlobalSusCause(continuousGlobalSusCause inCause);
    void RemoveContinuousGlobalSusCause(continuousGlobalSusCause inCause);

    float GetGlobalSusMeter()
    {
        return mGlobalSusMeter;
    };

    SuspicionMetre::SusBreakpoint GetLocalSusMetreBreakpoint(int playerNo) {
        return SuspicionMetre::getSusBreakpoint(mGlobalSusMeter);
    }

    void Update(float dt);
private:
    std::map<instantGlobalSusCause, float>  mInstantCauseSusSeverityMap =
    {
        {allarmTriggered,3}, {flagCaptured, 2}
    };

    std::map<continuousGlobalSusCause, float>  mContinuousCauseSusSeverityMap =
    {
        {passiveRecovery, -1}
    };

    float mGlobalSusMeter;
    float mGlobalRecoveryCooldown;
    std::vector<continuousGlobalSusCause> mContinuousGlobalSusCauseVector;

    void ChangePlayerGlobalSusMetre(float amount);
};

