#include "Vent.h"

#include "GameServer.h"
#include "NetworkObject.h"
#include "../CSC8503/LevelManager.h"
#include "PlayerObject.h"
#include "../CSC8503/DebugNetworkedGame.h"
#include "../CSC8503/SceneManager.h"

using namespace NCL::CSC8503;

Vent::Vent() {
	mIsOpen = false;
	mConnectedVent = nullptr;
	mInteractable = true;
	mInteractableItemType = InteractableItems::InteractableVents;
}

void Vent::ConnectVent(Vent* vent) {
	if (vent == nullptr) return;
	mConnectedVent = vent;
}

void Vent::HandlePlayerUse(GameObject* userObj) {
	if (mIsOpen) {
		auto* playerToTeleport = userObj;
		Transform& playerTransform = playerToTeleport->GetTransform();
		const Vector3& playerPos = playerToTeleport->GetTransform().GetPosition();

	const Vector3& teleportPos = mConnectedVent->GetTransform().GetPosition();
	const Quaternion& teleportOrient = mConnectedVent->GetTransform().GetOrientation();
	const Vector3 newPlayerPos = teleportPos + (teleportOrient * Vector3(5, 0, 0));


		playerTransform.SetPosition(newPlayerPos);
		playerTransform.SetOrientation(teleportOrient);
		LevelManager::GetLevelManager()->GetGameWorld()->GetMainCamera().SetYaw(mTransform.GetOrientation().ToEuler().y);
		SetIsOpen(false, true);
	}
}

void Vent::HandleItemUse(GameObject* userObj) {
	if (!mIsOpen) {
		auto* playerComp = static_cast<PlayerObject*>(userObj);
		if (playerComp != nullptr) {

			PlayerInventory::item usedItem = playerComp->GetEquippedItem();

			switch (usedItem) {
			case InventoryBuffSystem::PlayerInventory::screwdriver:
				SetIsOpen(true, true);
				break;
			default:
				break;
			}
		}
	}
}

void Vent::SyncVentStatusInMultiplayer() const {
# ifdef USEGL
	auto* sceneManager = SceneManager::GetSceneManager();
	DebugNetworkedGame* networkedGame = static_cast<DebugNetworkedGame*>(sceneManager->GetCurrentScene());
	if (networkedGame) {
		auto* networkObj = GetNetworkObject();
		if (networkObj) {
			int networkId = networkObj->GetnetworkID();

			SyncInteractablePacket packet(networkId, mIsOpen, mInteractableItemType);
			networkedGame->GetServer()->SendGlobalPacket(packet);
		}
	}
#endif
}

void Vent::SetIsOpen(bool isOpen, bool isSettedByServer) {
	mIsOpen = isOpen;

	bool isMultiplayerGame = !SceneManager::GetSceneManager()->IsInSingleplayer();
	if (isMultiplayerGame && isSettedByServer) {
		SyncVentStatusInMultiplayer();
	}
}

void Vent::Interact(InteractType interactType, GameObject* interactedObject) {

	switch (interactType) {
	case Use:
		HandlePlayerUse(interactedObject);
		break;
	case ItemUse:
		HandleItemUse(interactedObject);
		break;
	default:
		break;
	}
}

bool Vent::CanBeInteractedWith(InteractType interactType, GameObject* interactedObject){
	switch (interactType) {
	case Use:
		return mIsOpen;
		break;
	case ItemUse:
		return CanUseItem() && !mIsOpen;
		break;
	default:
		return false;
		break;
	}
}

bool Vent::CanUseItem(){
	auto* localPlayer = LevelManager::GetLevelManager()->GetTempPlayer();
	PlayerInventory::item usedItem = localPlayer->GetEquippedItem();

	switch (usedItem) {
	case InventoryBuffSystem::PlayerInventory::screwdriver:
		return true;
		break;
	default:
		return false;
		break;
	}
}
