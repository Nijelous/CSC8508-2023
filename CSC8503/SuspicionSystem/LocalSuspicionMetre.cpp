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

    Debug::Print("Sus:" + std::to_string(GetLocalSusMetreValue(0)), Vector2(70, 90));
    Debug::Print(std::to_string(mRecoveryCooldowns[0]), Vector2(70, 95));
}

void LocalSuspicionMetre::ChangePlayerLocalSusMetre(int playerNo, float ammount){
    mPlayerMeters[playerNo] += ammount;
    mPlayerMeters[playerNo] = std::clamp(mPlayerMeters[playerNo],
        mGlobalSusMeterPTR->GetGlobalSusMeter(),
        100.0f);

    if (ammount > 0)
        mRecoveryCooldowns[playerNo] = DT_UNTIL_LOCAL_RECOVERY;

}
