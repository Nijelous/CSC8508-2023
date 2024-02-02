#include "GlobalSuspicionMetre.h"
#include <algorithm>

void GlobalSuspicionMetre::AddInstantGlobalSusCause(instantGlobalSusCause inCause)
{
    ChangePlayerGlobalSusMetre(mInstantCauseSusSeverityMap[inCause]);
}

bool GlobalSuspicionMetre::AddActiveGlobalSusCause(activeGlobalSusCause inCause)
{
    auto foundCause = std::find(mActiveGlobalSusCauseVector.begin(), mActiveGlobalSusCauseVector.end(), inCause);

    //If the foundCause is not already in the mActiveGlobalSusCauseVector vector of that player
    if (foundCause == mActiveGlobalSusCauseVector.end())
    {
        mActiveGlobalSusCauseVector.push_back(inCause);
        return true;
    }

    return false;
}

bool GlobalSuspicionMetre::RemoveActiveGlobalSusCause(activeGlobalSusCause inCause)
{
    auto foundCause = std::find(mActiveGlobalSusCauseVector.begin(), mActiveGlobalSusCauseVector.end(), inCause);

    //If the foundCause is not already in the mActiveGlobalSusCauseVector vector of that player
    if (foundCause == mActiveGlobalSusCauseVector.end())
    {
        mActiveGlobalSusCauseVector.erase(foundCause);
        return true;
    }
    return false;
};

void GlobalSuspicionMetre::Update(float dt)
{
    for (activeGlobalSusCause thisCause : mActiveGlobalSusCauseVector)
    {
        ChangePlayerGlobalSusMetre(mActiveCauseSusSeverityMap[thisCause]);
    }

    if (mGlobalRecoveryCooldown == DT_UNTIL_GlOBAL_RECOVERY)
        RemoveActiveGlobalSusCause(passiveRecovery);

    mGlobalRecoveryCooldown -= dt;
    mGlobalRecoveryCooldown = std::max(mGlobalRecoveryCooldown, 0.0f);

    if (mGlobalRecoveryCooldown == 0)
        AddActiveGlobalSusCause(passiveRecovery);
}

void GlobalSuspicionMetre::ChangePlayerGlobalSusMetre(float ammount)
{
    mGlobalSusMeter += ammount;
    mGlobalSusMeter = std::clamp(mGlobalSusMeter,
        0.0f,
        100.0f);

    if (ammount < 0)
        mGlobalRecoveryCooldown = DT_UNTIL_GlOBAL_RECOVERY;
}
;
