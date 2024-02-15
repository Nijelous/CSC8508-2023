#include "LocalSuspicionMetre.h"
#include <algorithm>

using namespace SuspicionSystem;

void LocalSuspicionMetre::Init(){
    for (int i = 0; i < NCL::CSC8503::MAX_PLAYERS; i++)
    {
        mPlayerMeters[i] = 0;
        mRecoveryCooldowns[i] = 0;
        mActiveLocalSusCauseVector[i].clear();
    }
}

void LocalSuspicionMetre::AddInstantLocalSusCause(instantLocalSusCause inCause, int playerNo){
    ChangePlayerLocalSusMetre(playerNo, mInstantLocalSusCauseSeverityMap[inCause]);
};

bool LocalSuspicionMetre::AddActiveLocalSusCause(activeLocalSusCause inCause, int playerNo){
    auto foundCause = std::find(mActiveLocalSusCauseVector[playerNo].begin(), mActiveLocalSusCauseVector[playerNo].end(), inCause);

    //If the foundCause is not already in the activeSusCauses vector of that player
    if (foundCause == mActiveLocalSusCauseVector[playerNo].end())
    {
        mActiveLocalSusCauseVector[playerNo].push_back(inCause);
        return true;
    }

    return false;
};

bool LocalSuspicionMetre::RemoveActiveLocalSusCause(activeLocalSusCause inCause, int playerNo){
    auto foundCause = std::find(mActiveLocalSusCauseVector[playerNo].begin(), mActiveLocalSusCauseVector[playerNo].end(), inCause);

    //If the foundCause is not already int the activeSusCauses vector of that player
    if (foundCause != mActiveLocalSusCauseVector[playerNo].end())
    {
        mActiveLocalSusCauseVector[playerNo].erase(foundCause);
        return true;
    }

    return false;
}

void LocalSuspicionMetre::UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo){
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

void LocalSuspicionMetre::Update(float dt){
    for (int playerNo = 0; playerNo < NCL::CSC8503::MAX_PLAYERS; playerNo++)
    {
        for (activeLocalSusCause thisCause : mActiveLocalSusCauseVector[playerNo])
        {
            ChangePlayerLocalSusMetre(playerNo, mActiveLocalSusCauseSeverityMap[thisCause]);
        }

        if (mRecoveryCooldowns[playerNo] == DT_UNTIL_LOCAL_RECOVERY)
            RemoveActiveLocalSusCause(passiveRecovery, playerNo);

        mRecoveryCooldowns[playerNo] -= dt;
        mRecoveryCooldowns[playerNo] = std::max(mRecoveryCooldowns[playerNo], 0.0f);

        if (mRecoveryCooldowns[playerNo] == 0)
            AddActiveLocalSusCause(passiveRecovery, playerNo);
    }
}

void LocalSuspicionMetre::ChangePlayerLocalSusMetre(int playerNo, float ammount){
    mPlayerMeters[playerNo] += ammount;
    mPlayerMeters[playerNo] = std::clamp(2.0f,
        mGlobalSusMeterPTR->GetGlobalSusMeter(),
        100.0f);

    if (ammount < 0)
        mRecoveryCooldowns[playerNo] = DT_UNTIL_LOCAL_RECOVERY;
}
