#include "GlobalSuspicionMetre.h"

#include <algorithm>

#include "../DebugNetworkedGame.h"
#include "../SceneManager.h"

using namespace SuspicionSystem;
using namespace NCL::CSC8503;

void SuspicionSystem::GlobalSuspicionMetre::Init(){
    mGlobalSusMeter = 0;
    mGlobalRecoveryCooldown = 0;
}

void GlobalSuspicionMetre::AddInstantGlobalSusCause(const instantGlobalSusCause &inCause){
    ChangePlayerGlobalSusMetre(mInstantCauseSusSeverityMap[inCause]);
    HandleGlobalSusChangeNetworking(mGlobalSusMeter);
    Notify(SuspicionMetre::GetSusBreakpoint(mGlobalSusMeter));
}

void GlobalSuspicionMetre::AddContinuousGlobalSusCause(const continuousGlobalSusCause &inCause){
    auto foundCause = std::find(mContinuousGlobalSusCauseVector.begin(), mContinuousGlobalSusCauseVector.end(), inCause);

    if (foundCause == mContinuousGlobalSusCauseVector.end())
    {
        mContinuousGlobalSusCauseVector.push_back(inCause);
    }
}

void GlobalSuspicionMetre::RemoveContinuousGlobalSusCause(const continuousGlobalSusCause &inCause){
    auto foundCause = std::find(mContinuousGlobalSusCauseVector.begin(), mContinuousGlobalSusCauseVector.end(), inCause);

    if (foundCause != mContinuousGlobalSusCauseVector.end())
    {
        mContinuousGlobalSusCauseVector.erase(foundCause);
    }
}
void GlobalSuspicionMetre::SetMinGlobalSusMetre(const instantGlobalSusCause& inCause){
    SetMinGlobalSusMetre(mInstantCauseSusSeverityMap[inCause]);
    HandleGlobalSusChangeNetworking(mGlobalSusMeter);
    Notify(SuspicionMetre::GetSusBreakpoint(mGlobalSusMeter));
}
;

void GlobalSuspicionMetre::Attach(GlobalSuspicionObserver* observer) {
    mGlobalSuspicionObserverList.push_back(observer);
}

void GlobalSuspicionMetre::Detach(GlobalSuspicionObserver* observer) {
    mGlobalSuspicionObserverList.remove(observer);
}

void GlobalSuspicionMetre::Notify(const SuspicionMetre::SusBreakpoint susBreakpoint) {
    std::list<GlobalSuspicionObserver*>::iterator iterator = mGlobalSuspicionObserverList.begin();
    while (iterator != mGlobalSuspicionObserverList.end()) {
        (*iterator)->UpdateGlobalSuspicionObserver(susBreakpoint);
        ++iterator;
    }
}

void GlobalSuspicionMetre::Update(float dt){
    if (mGlobalSusMeter == 0.0f &&
        mContinuousGlobalSusCauseVector.size() == 0)
        return;

    float unChangedSusValue = mGlobalSusMeter;

    for (continuousGlobalSusCause thisCause : mContinuousGlobalSusCauseVector)
    {
        ChangePlayerGlobalSusMetre(mContinuousCauseSusSeverityMap[thisCause]);
    }

    if (mGlobalSusMeter <= unChangedSusValue) {
        if (mGlobalRecoveryCooldown == 0.0f)
            ChangePlayerGlobalSusMetre(mContinuousCauseSusSeverityMap[passiveRecovery] * dt);
        else
            mGlobalRecoveryCooldown = std::max(mGlobalRecoveryCooldown - dt, 0.0f);
    }
    else {
        mGlobalRecoveryCooldown = DT_UNTIL_GlOBAL_RECOVERY;
    }

    if ((int)(mGlobalSusMeter) != (int)(unChangedSusValue))
        HandleGlobalSusChangeNetworking(mGlobalSusMeter);
}

void GlobalSuspicionMetre::ChangePlayerGlobalSusMetre(float amount){
    mGlobalSusMeter += amount;
    mGlobalSusMeter = std::clamp(mGlobalSusMeter,
        0.0f,
        100.0f);
}

void GlobalSuspicionMetre::SetMinGlobalSusMetre(float amount) {
    mGlobalSusMeter = std::max(amount, mGlobalSusMeter);
}

void GlobalSuspicionMetre::HandleGlobalSusChangeNetworking(const int& changedValue){
#ifdef USEGL
    int localPlayerId = 0;
    DebugNetworkedGame* game = reinterpret_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
    if (!SceneManager::GetSceneManager()->IsInSingleplayer()) {
        const bool isServer = game->GetIsServer();
        if (isServer) {
            game->SendClientSyncGlobalSusChangePacket(changedValue);
        }
    }
#endif
};

void GlobalSuspicionMetre::SyncSusChange(int changedValue) {
    mGlobalSusMeter = changedValue;
}