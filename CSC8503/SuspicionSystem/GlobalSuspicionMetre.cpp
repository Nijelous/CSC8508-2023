#include "GlobalSuspicionMetre.h"
#include <algorithm>

using namespace SuspicionSystem;

void SuspicionSystem::GlobalSuspicionMetre::Init()
{
    mGlobalSusMeter = 0;
    mGlobalRecoveryCooldown = 0;
}

void GlobalSuspicionMetre::AddInstantGlobalSusCause(instantGlobalSusCause inCause)
{
    ChangePlayerGlobalSusMetre(mInstantCauseSusSeverityMap[inCause]);
}

void GlobalSuspicionMetre::AddContinuousGlobalSusCause(continuousGlobalSusCause inCause)
{
    auto foundCause = std::find(mContinuousGlobalSusCauseVector.begin(), mContinuousGlobalSusCauseVector.end(), inCause);

    if (foundCause == mContinuousGlobalSusCauseVector.end())
    {
        mContinuousGlobalSusCauseVector.push_back(inCause);
    }
}

void GlobalSuspicionMetre::RemoveContinuousGlobalSusCause(continuousGlobalSusCause inCause)
{
    auto foundCause = std::find(mContinuousGlobalSusCauseVector.begin(), mContinuousGlobalSusCauseVector.end(), inCause);

    if (foundCause != mContinuousGlobalSusCauseVector.end())
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
    mGlobalSusMeter += amount;
    mGlobalSusMeter = std::clamp(mGlobalSusMeter,
        0.0f,
        100.0f);

    if (amount < 0)
        mGlobalRecoveryCooldown = DT_UNTIL_GlOBAL_RECOVERY;
};
