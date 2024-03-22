#include "LocalSuspicionMetre.h"

#include <algorithm>

#include "NetworkObject.h"
#include "../DebugNetworkedGame.h"
#include "../SceneManager.h"

#include "../LevelManager.h"

using namespace SuspicionSystem;

void LocalSuspicionMetre::Init(){
    for (int i = 0; i < NCL::CSC8503::MAX_PLAYERS; i++)
    {
        mPlayerMeters[i] = 0;
        mRecoveryCooldowns[i] = DT_UNTIL_LOCAL_RECOVERY;
        mActiveLocalSusCauseVector[i].clear();
        mActiveLocalSusCausesToRemove[i].clear();
    }
}

void LocalSuspicionMetre::AddInstantLocalSusCause(const instantLocalSusCause &inCause, const int &playerNo){
    const float guardDist = LevelManager::GetLevelManager()->GetNearestGuardToPlayerDistance(playerNo);
    const float guardDistPerc = GetPercentageBasedOnDistance(guardDist);

    ChangePlayerLocalSusMetre(playerNo, mInstantLocalSusCauseSeverityMap[inCause] * guardDistPerc);
    HandleLocalSusChangeNetworking(mPlayerMeters[playerNo], playerNo);
};

bool SuspicionSystem::LocalSuspicionMetre::IsActiveSusCauseForPlayer(const activeLocalSusCause& inCause, const int& playerNo)
{
    return std::find(mActiveLocalSusCauseVector[playerNo].begin(), mActiveLocalSusCauseVector[playerNo].end(), inCause)
        != mActiveLocalSusCauseVector[playerNo].end();
}

void LocalSuspicionMetre::AddActiveLocalSusCause(const activeLocalSusCause &inCause, const int &playerNo){
    if (!IsActiveSusCauseForPlayer(inCause,playerNo))
    {
        mActiveLocalSusCauseVector[playerNo].push_back(inCause);
        HandleActiveSusCauseNetworking(inCause, playerNo, true);
    }
};

void LocalSuspicionMetre::RemoveActiveLocalSusCause(const activeLocalSusCause &inCause, const int &playerNo){
    if (IsActiveSusCauseForPlayer(inCause, playerNo))
    {
        mActiveLocalSusCausesToRemove[playerNo].push_back(inCause);
        HandleActiveSusCauseNetworking(inCause, playerNo, false);
    }
}

double SuspicionSystem::LocalSuspicionMetre::GetPercentageBasedOnDistance(const float distance, const float minVal, const float minDist, const float maxDist){
    float outVal;
    if (distance < minDist)
        return 1;

    if (distance > maxDist)
        return minVal / 100;

    outVal = (std::pow((maxDist - distance) / (maxDist - minDist),2)
              * (100 - minVal)) + minVal;

    outVal = std::clamp(outVal,
                    minVal,
                    100.0f);

    return outVal / 100;
}

void LocalSuspicionMetre::HandleActiveSusCauseNetworking(const activeLocalSusCause& inCause, const int& playerNo, const bool& toApply){
#ifdef USEGEL
    int localPlayerId = 0;
    DebugNetworkedGame* game = reinterpret_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
    if (!SceneManager::GetSceneManager()->IsInSingleplayer()) {
        const auto* localPlayer = game->GetLocalPlayer();
        localPlayerId = localPlayer->GetPlayerID();

        const bool isServer = game->GetIsServer();
        if (isServer) {
            game->SendClientSyncLocalActiveSusCausePacket(playerNo, inCause, toApply);
        }
    }
#endif
}


void LocalSuspicionMetre::UpdateGlobalSuspicionObserver(SuspicionMetre::SusBreakpoint susBreakpoint){
    for (int i = 0; i < NCL::CSC8503::MAX_PLAYERS; i++)
    {
        ChangePlayerLocalSusMetre(i, 0);
        HandleLocalSusChangeNetworking(mPlayerMeters[i], i);
    }
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
            mActiveLocalSusCauseVector[playerNo].size() > 0) {
            float tempSusAmount = 0;

            const float guardDist = LevelManager::GetLevelManager()->GetNearestGuardToPlayerDistance(playerNo);
            const float guardDistPerc = GetPercentageBasedOnDistance(guardDist);

            for (activeLocalSusCause thisCause : mActiveLocalSusCauseVector[playerNo]) {
                tempSusAmount += mActiveLocalSusCauseSeverityMap[thisCause] * dt * guardDistPerc;
            }

            ChangePlayerLocalSusMetre(playerNo,tempSusAmount);

            if (tempSusAmount <= 0){
                if (mRecoveryCooldowns[playerNo] == 0.0f)
                    ChangePlayerLocalSusMetre(playerNo, mActiveLocalSusCauseSeverityMap[passiveRecovery] * dt);
                else
                    mRecoveryCooldowns[playerNo] = std::max(mRecoveryCooldowns[playerNo] - dt, 0.0f);
            }
            else {
                mRecoveryCooldowns[playerNo] = DT_UNTIL_LOCAL_RECOVERY;
            }

            if ((int)(tempSusAmount) != 0)
                HandleLocalSusChangeNetworking(mPlayerMeters[playerNo], playerNo);
        }

        for(const activeLocalSusCause activeSusCause:mActiveLocalSusCausesToRemove[playerNo])
            mActiveLocalSusCauseVector[playerNo].erase(std::remove(
            mActiveLocalSusCauseVector[playerNo].begin(), mActiveLocalSusCauseVector[playerNo].end(), activeSusCause),
            mActiveLocalSusCauseVector[playerNo].end());
    }
    mActiveLocalSusCausesToRemove->clear();
}

void LocalSuspicionMetre::SyncActiveSusCauses(int playerID, int localPlayerID, activeLocalSusCause inCause, bool toApply){
    if (localPlayerID != playerID)
        return;

    if (toApply)
        AddActiveLocalSusCause(inCause, playerID);
    else
        RemoveActiveLocalSusCause(inCause, playerID);
}

void LocalSuspicionMetre::ChangePlayerLocalSusMetre(const int &playerNo, const float &ammount){
    mPlayerMeters[playerNo] = std::clamp(mPlayerMeters[playerNo] + ammount,
                                        mGlobalSusMeterPTR->GetGlobalSusMeter(),
                                        99.0f);
}

void LocalSuspicionMetre::HandleLocalSusChangeNetworking(const int& changedValue, const int& playerNo) {
#ifdef USEGL
    int localPlayerId = 0;
    DebugNetworkedGame* game = reinterpret_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
    if (!SceneManager::GetSceneManager()->IsInSingleplayer()) {
        const auto* localPlayer = game->GetLocalPlayer();
        localPlayerId = localPlayer->GetPlayerID();

        const bool isServer = game->GetIsServer();
        if (isServer) {
            game->SendClientSyncLocalSusChangePacket(playerNo, changedValue);
        }
    }
#endif
}

void LocalSuspicionMetre::SyncSusChange(int playerID, int localPlayerID, int changedValue){
    if (localPlayerID != playerID)
        return;
    mPlayerMeters[playerID] = changedValue;
}