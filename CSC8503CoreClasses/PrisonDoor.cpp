#include "PrisonDoor.h"
#include "GameObject.h"
#include "../CSC8503/LevelManager.h"
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
		DebugNetworkedGame* networkedGame = static_cast<DebugNetworkedGame*>(sceneManager->GetCurrentScene());
		const bool isServer = networkedGame->GetIsServer();
		if (isServer)
			SyncInteractableDoorStatusInMultiplayer(toOpen);
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

void PrisonDoor::SetNavMeshFlags(int flag) {
	float* pos = new float[3] { mTransform.GetPosition().x, mTransform.GetPosition().y, mTransform.GetPosition().z };
	AABBVolume* volume = (AABBVolume*)mBoundingVolume;
	float* halfSize = new float[3] { volume->GetHalfDimensions().x, volume->GetHalfDimensions().y, volume->GetHalfDimensions().z };
	LevelManager::GetLevelManager()->LoadDoorInNavGrid(pos, halfSize, (PolyFlags)flag);
}