#include "Vent.h"

#include "GameServer.h"
#include "NetworkObject.h"
#include "../CSC8503/LevelManager.h"
#include "../CSC8503/DebugNetworkedGame.h"
#include "../CSC8503/SceneManager.h"
#include "GameClient.h"

using namespace NCL::CSC8503;

Vent::Vent() {
	mName = "Vent";
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
#ifdef USEGL
		this->GetSoundObject()->TriggerSoundEvent();
		mConnectedVent->GetSoundObject()->TriggerSoundEvent();
#endif
		LevelManager::GetLevelManager()->GetGameWorld()->GetMainCamera().SetYaw(mTransform.GetOrientation().ToEuler().y);
#ifdef PROSPERO
		SetIsOpen(false, true);
#endif
#ifdef USEGL
		DebugNetworkedGame* networkedGame = static_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
		if (networkedGame) {
			SetIsOpen(false, networkedGame->GetIsServer());
		}
		else {
			SetIsOpen(false, true);
		}
#endif
	}
}

void Vent::HandleItemUse(GameObject* userObj) {
	if (!mIsOpen) {
		auto* playerComp = static_cast<PlayerObject*>(userObj);
		if (playerComp != nullptr) {

			PlayerInventory::item usedItem = playerComp->GetEquippedItem();

			switch (usedItem) {
			case InventoryBuffSystem::PlayerInventory::screwdriver:
#ifdef PROSPERO
				SetIsOpen(true, true);
#endif
#ifdef USEGL
				{
					DebugNetworkedGame* networkedGame = static_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
					if (networkedGame) {
						SetIsOpen(true, networkedGame->GetIsServer());
					}
					else {
						SetIsOpen(true, true);
					}
				}
#endif
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
	if (mIsOpen == true) {
#ifdef USEGL
		this->GetSoundObject()->LockDoorTriggered();
#endif
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
