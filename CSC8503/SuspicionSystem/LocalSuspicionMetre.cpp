#include "LocalSuspicionMetre.h"
#include <algorithm>
#include "Debug.h"
#include <string>

using namespace SuspicionSystem;

void LocalSuspicionMetre::Init(){
    for (int i = 0; i < NCL::CSC8503::MAX_PLAYERS; i++)
    {
        mPlayerMeters[i] = 0;
        mRecoveryCooldowns[i] = 0;
        mActiveLocalSusCauseVector[i].clear();
    }
}

void LocalSuspicionMetre::AddInstantLocalSusCause(const instantLocalSusCause &inCause, const int &playerNo){
    ChangePlayerLocalSusMetre(playerNo, mInstantLocalSusCauseSeverityMap[inCause]);
};

bool LocalSuspicionMetre::AddActiveLocalSusCause(const activeLocalSusCause &inCause, const int &playerNo){
    auto foundCause = std::find(mActiveLocalSusCauseVector[playerNo].begin(), mActiveLocalSusCauseVector[playerNo].end(), inCause);

    //If the foundCause is not already in the activeSusCauses vector of that player
    if (foundCause == mActiveLocalSusCauseVector[playerNo].end())
    {
        mActiveLocalSusCauseVector[playerNo].push_back(inCause);
        return true;
    }

    return false;
};

bool LocalSuspicionMetre::RemoveActiveLocalSusCause(const activeLocalSusCause &inCause, const int &playerNo){
    auto foundCause = std::find(mActiveLocalSusCauseVector[playerNo].begin(), mActiveLocalSusCauseVector[playerNo].end(), inCause);

    //If the foundCause is not already int the activeSusCauses vector of that player
    if (foundCause != mActiveLocalSusCauseVector[playerNo].end())
    {
        mActiveLocalSusCauseVector[playerNo].erase(foundCause);
        return true;
    }

    return false;
}

void LocalSuspicionMetre::UpdatePlayerBuffsObserver(const BuffEvent buffEvent, const int playerNo){
    switch (buffEvent)
    {
    case disguiseBuffApplied:
        AddActiveLocalSusCause(disguiseBuff, playerNo);
        break;
    case disguiseBuffRemoved:
        RemoveActiveLocalSusCause(disguiseBuff, playerNo);
        break;
    case playerMakesSound:
        AddInstantLocalSusCause(soundEmitted, playerNo);
        break;
    default:
        break;
    }
}

void LocalSuspicionMetre::Update(float dt) {
    for (int playerNo = 0; playerNo < NCL::CSC8503::MAX_PLAYERS; playerNo++)
    {
        if (GetLocalSusMetreValue(playerNo) != 0.0f ||
            mActiveLocalSusCauseVector[playerNo].size() > 1)
        {
            for (activeLocalSusCause thisCause : mActiveLocalSusCauseVector[playerNo])
            {
                ChangePlayerLocalSusMetre(playerNo, mActiveLocalSusCauseSeverityMap[thisCause] * dt);
            }

            if (mRecoveryCooldowns[playerNo] == DT_UNTIL_LOCAL_RECOVERY)
                RemoveActiveLocalSusCause(passiveRecovery, playerNo);
        }
        mRecoveryCooldowns[playerNo] = std::max(mRecoveryCooldowns[playerNo] - dt, 0.0f);

        if (mRecoveryCooldowns[playerNo] == 0.0f)
            AddActiveLocalSusCause(passiveRecovery, playerNo);

    }

}

void LocalSuspicionMetre::ChangePlayerLocalSusMetre(const int &playerNo, const float &ammount){
    mPlayerMeters[playerNo] += ammount;
    mPlayerMeters[playerNo] = std::clamp(mPlayerMeters[playerNo],
        mGlobalSusMeterPTR->GetGlobalSusMeter(),
        100.0f);

    if (ammount > 0)
        mRecoveryCooldowns[playerNo] = DT_UNTIL_LOCAL_RECOVERY;

}
