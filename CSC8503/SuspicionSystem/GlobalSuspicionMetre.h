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
        allarmTriggered, flagCaptured
    };

    const enum activeGlobalSusCause
    {
        passiveRecovery
    };

    void AddInstantGlobalSusCause(instantGlobalSusCause inCause);

    bool AddActiveGlobalSusCause(activeGlobalSusCause inCause);
    bool RemoveActiveGlobalSusCause(activeGlobalSusCause inCause);

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

    std::map<activeGlobalSusCause, float>  mActiveCauseSusSeverityMap =
    {
        {passiveRecovery, -1}
    };

    float mGlobalSusMeter;
    float mGlobalRecoveryCooldown;
    std::vector<activeGlobalSusCause> mActiveGlobalSusCauseVector;

    void ChangePlayerGlobalSusMetre(float ammount);
};

