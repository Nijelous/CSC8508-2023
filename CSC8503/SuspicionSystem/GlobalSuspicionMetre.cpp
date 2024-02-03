#include "GlobalSuspicionMetre.h"
#include <algorithm>

void GlobalSuspicionMetre::AddInstantGlobalSusCause(instantGlobalSusCause inCause)
{
    ChangePlayerGlobalSusMetre(mInstantCauseSusSeverityMap[inCause]);
}

bool GlobalSuspicionMetre::AddContinuousGlobalSusCause(continuousGlobalSusCause inCause)
{
    auto* foundCause = std::find(mContinuousGlobalSusCauseVector.begin(), mContinuousGlobalSusCauseVector.end(), inCause);

    if (*foundCause == mContinuousGlobalSusCauseVector.end())
    {
        mContinuousGlobalSusCauseVector.push_back(inCause);
    }
}

void GlobalSuspicionMetre::RemoveContinuousGlobalSusCause(continuousGlobalSusCause inCause)
{
    auto* foundCause = std::find(mContinuousGlobalSusCauseVector.begin(), mContinuousGlobalSusCauseVector.end(), inCause);

    if (*foundCause != mContinuousGlobalSusCauseVector.end())
    {
        mContinuousGlobalSusCauseVector.erase(foundCause);
    }
};

void GlobalSuspicionMetre::Update(float dt)
{
    for (continuousGlobalSusCause thisCause : mContinuousGlobalSusCauseVector)
    {
        ChangePlayerGlobalSusMetre(mContinuousCauseSusSeverityMap[thisCause]);
    }

    if (mGlobalRecoveryCooldown == DT_UNTIL_GlOBAL_RECOVERY)
        RemoveContinuousGlobalSusCause(passiveRecovery);

    mGlobalRecoveryCooldown -= dt;
    mGlobalRecoveryCooldown = std::max(mGlobalRecoveryCooldown, 0.0f);

    if (mGlobalRecoveryCooldown == 0)
        AddContinuousGlobalSusCause(passiveRecovery);
}

void GlobalSuspicionMetre::ChangePlayerGlobalSusMetre(float amount)
{
    mGlobalSusMeter += ammount;
    mGlobalSusMeter = std::clamp(mGlobalSusMeter,
        0.0f,
        100.0f);

    if (ammount < 0)
        mGlobalRecoveryCooldown = DT_UNTIL_GlOBAL_RECOVERY;
};
