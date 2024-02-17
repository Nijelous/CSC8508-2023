#include "Vent.h"
#include "../CSC8503/LevelManager.h"
#include "PlayerObject.h"

using namespace NCL::CSC8503;

Vent::Vent() {
	mIsOpen = false;
	mConnectedVent = nullptr;
}

void Vent::ConnectVent(Vent* vent) {
	if (vent == nullptr) return;
	mConnectedVent = vent;
}

void NCL::CSC8503::Vent::HandleItemUse() {
	if (!mIsOpen) {
		auto* localPlayer = LevelManager::GetLevelManager()->GetTempPlayer();
		PlayerInventory::item usedItem = localPlayer->GetEquippedItem();

		switch (usedItem) {
		case InventoryBuffSystem::PlayerInventory::screwdriver:
			mIsOpen = true;
			break;
		default:
			break;
		}
	}
}

void NCL::CSC8503::Vent::HandlePlayerUse() {
	if (mIsOpen) {
		auto* localPlayer = (GameObject*)(LevelManager::GetLevelManager()->GetTempPlayer());
		Transform& playerTransform = localPlayer->GetTransform();
		const Vector3& playerPos = localPlayer->GetTransform().GetPosition();

		const Vector3& teleportPos = mConnectedVent->GetTransform().GetPosition();
		const Vector3 newPlayerPos = Vector3(teleportPos.x + 5, playerPos.y, teleportPos.z);

		playerTransform.SetPosition(newPlayerPos);
		mIsOpen = false;
	}
}

void Vent::Interact(NCL::CSC8503::InteractType interactType) {

	switch (interactType)
	{
	case NCL::CSC8503::Use:
		HandlePlayerUse();
		break;
	case NCL::CSC8503::ItemUse:
		HandleItemUse();
		break;
	default:
		break;
	}
}
