#include "GlobalSuspicionMetre.h"
#include <algorithm>

using namespace SuspicionSystem;

void SuspicionSystem::GlobalSuspicionMetre::Init(){
    mGlobalSusMeter = 0;
    mGlobalRecoveryCooldown = 0;
}

void GlobalSuspicionMetre::AddInstantGlobalSusCause(instantGlobalSusCause inCause){
    ChangePlayerGlobalSusMetre(mInstantCauseSusSeverityMap[inCause]);
}

void GlobalSuspicionMetre::AddContinuousGlobalSusCause(continuousGlobalSusCause inCause){
    auto foundCause = std::find(mContinuousGlobalSusCauseVector.begin(), mContinuousGlobalSusCauseVector.end(), inCause);

    if (foundCause == mContinuousGlobalSusCauseVector.end())
    {
        mContinuousGlobalSusCauseVector.push_back(inCause);
    }
}

void GlobalSuspicionMetre::RemoveContinuousGlobalSusCause(continuousGlobalSusCause inCause){
    auto foundCause = std::find(mContinuousGlobalSusCauseVector.begin(), mContinuousGlobalSusCauseVector.end(), inCause);

    if (foundCause != mContinuousGlobalSusCauseVector.end())
    {
        mContinuousGlobalSusCauseVector.erase(foundCause);
    }
};

void GlobalSuspicionMetre::Attach(GlobalSuspicionObserver* observer) {
    mGlobalSuspicionObserverList.push_back(observer);
}

void GlobalSuspicionMetre::Detach(GlobalSuspicionObserver* observer) {
    mGlobalSuspicionObserverList.remove(observer);
}

void GlobalSuspicionMetre::Notify(SuspicionMetre::SusBreakpoint susBreakpoint) {
    std::list<GlobalSuspicionObserver*>::iterator iterator = mGlobalSuspicionObserverList.begin();
    while (iterator != mGlobalSuspicionObserverList.end()) {
        (*iterator)->UpdateGlobalSuspicionObserver(susBreakpoint);
        ++iterator;
    }
}

void GlobalSuspicionMetre::Update(float dt){

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

void GlobalSuspicionMetre::ChangePlayerGlobalSusMetre(float amount){
    SuspicionMetre::SusBreakpoint tempBreakpoint = SuspicionMetre::GetSusBreakpoint(mGlobalSusMeter);
    mGlobalSusMeter += amount;
    mGlobalSusMeter = std::clamp(mGlobalSusMeter,
        0.0f,
        100.0f);
    if (SuspicionMetre::GetSusBreakpoint(mGlobalSusMeter) != tempBreakpoint)
        Notify(SuspicionMetre::GetSusBreakpoint(mGlobalSusMeter));
    if (amount < 0)
        mGlobalRecoveryCooldown = DT_UNTIL_GlOBAL_RECOVERY;
};
