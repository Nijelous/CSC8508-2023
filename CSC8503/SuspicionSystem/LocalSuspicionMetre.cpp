#include "LocalSuspicionMetre.h"
#include <algorithm>
#include "Debug.h"
#include <string>
#include "NetworkObject.h"
#include "../DebugNetworkedGame.h"
#include "../SceneManager.h"
#include "../CSC8503/LevelManager.h"

namespace NCL::CSC8503
{
	class DebugNetworkedGame;
}

using namespace SuspicionSystem;

void LocalSuspicionMetre::Init(){
    for (int i = 0; i < NCL::CSC8503::MAX_PLAYERS; i++)
    {
        mPlayerMeters[i] = 0;
        mRecoveryCooldowns[i] = DT_UNTIL_LOCAL_RECOVERY;
        mActiveLocalSusCauseVector[i].clear();
    }
}

void LocalSuspicionMetre::AddInstantLocalSusCause(const instantLocalSusCause &inCause, const int &playerNo){
    ChangePlayerLocalSusMetre(playerNo, mInstantLocalSusCauseSeverityMap[inCause]);
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
            const float unChangedSusValue = mPlayerMeters[playerNo];

            for (activeLocalSusCause thisCause : mActiveLocalSusCauseVector[playerNo]) {
                ChangePlayerLocalSusMetre(playerNo, mActiveLocalSusCauseSeverityMap[thisCause] * dt);
            }

            if (mPlayerMeters[playerNo] <= unChangedSusValue){
                if (mRecoveryCooldowns[playerNo] == 0.0f)
                    ChangePlayerLocalSusMetre(playerNo, mActiveLocalSusCauseSeverityMap[passiveRecovery] * dt);
                else
                    mRecoveryCooldowns[playerNo] = std::max(mRecoveryCooldowns[playerNo] - dt, 0.0f);
            }
            else {
                mRecoveryCooldowns[playerNo] = DT_UNTIL_LOCAL_RECOVERY;
            }

            if ((int)(mPlayerMeters[playerNo]) != (int)(unChangedSusValue))
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
                                        100.0f);
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