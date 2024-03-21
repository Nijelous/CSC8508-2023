#include "PrisonDoor.h"
#include "GameObject.h"
#include "../CSC8503/LevelManager.h"
#include "NetworkObject.h"
#include "../CSC8503/DebugNetworkedGame.h"
#include "../CSC8503/SceneManager.h"
#include "Interactable.h"
using namespace NCL::CSC8503;

void PrisonDoor::UpdateGlobalSuspicionObserver(SuspicionSystem::SuspicionMetre::SusBreakpoint susBreakpoint){
	switch (susBreakpoint)
	{
	case SuspicionSystem::SuspicionMetre::high:
		SetIsOpen(false);
		break;
	default:
		break;
	}
}

void PrisonDoor::Open() {
	GetTransform().SetPosition(GetTransform().GetPosition() + Vector3(0, 7.25, 0));
	mTimer = -1;
	mIsOpen = true;
}

void PrisonDoor::Close() {
	GetTransform().SetPosition(GetTransform().GetPosition() + Vector3(0, -7.25, 0));
	SetNavMeshFlags(2);
	mIsOpen = false;
}

void PrisonDoor::UpdateObject(float dt) {
	if (!mIsOpen && mTimer > 0)
		CountDownTimer(dt);

	if (mTimer == 0)
		SetIsOpen(true);
}

void PrisonDoor::SetIsOpen(bool toOpen) {
	if(!toOpen)
		mTimer = initDoorTimer;
	if (toOpen == mIsOpen)
		return;

	auto* sceneManager = SceneManager::GetSceneManager();
	const bool isSingleplayer = sceneManager->IsInSingleplayer();
	if (!isSingleplayer)
	{
#ifdef USEGL
		DebugNetworkedGame* networkedGame = static_cast<DebugNetworkedGame*>(sceneManager->GetCurrentScene());
		const bool isServer = networkedGame->GetIsServer();
		if (isServer)
			SyncInteractableDoorStatusInMultiplayer(toOpen);
#endif
	}

	if (toOpen) {
		Open();
	}
	else {
		Close();
	}
}
#ifdef USEGL
void PrisonDoor::SyncInteractableDoorStatusInMultiplayer(bool toOpen){
	auto* sceneManager = SceneManager::GetSceneManager();
	DebugNetworkedGame* networkedGame = static_cast<DebugNetworkedGame*>(sceneManager->GetCurrentScene());
	if (networkedGame) {
		auto* networkObj = GetNetworkObject();
		if (networkObj) {
			const int networkId = networkObj->GetnetworkID();
			networkedGame->SendInteractablePacket(networkId, toOpen, InteractableDoors);
		}
	}
}

void PrisonDoor::SyncDoor(bool toOpen){
	SetIsOpen(toOpen);
}
#endif