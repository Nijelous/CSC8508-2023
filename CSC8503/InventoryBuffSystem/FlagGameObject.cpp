#include "FlagGameObject.h"
#include "PlayerInventory.h"
#include "PlayerObject.h"
#include "../LevelManager.h"
#include "../SuspicionSystem/GlobalSuspicionMetre.h"
#include "GameClient.h"
#include "Interactable.h"
#include "../CSC8503/DebugNetworkedGame.h"
#include "../CSC8503/SceneManager.h"
#include "../CSC8503/NetworkPlayer.h"
using namespace NCL;
using namespace CSC8503;

FlagGameObject::FlagGameObject(InventoryBuffSystemClass* inventoryBuffSystemClassPtr, SuspicionSystemClass* suspicionSystemClassPtr,
	std::map<GameObject*, int>* playerObjectToPlayerNoMap, int pointsWorth)
	: Item(PlayerInventory::item::flag, *inventoryBuffSystemClassPtr){
	mName = "Flag";
	mItemType = PlayerInventory::item::flag;
	mInventoryBuffSystemClassPtr = inventoryBuffSystemClassPtr;
	mSuspicionSystemClassPtr = suspicionSystemClassPtr;
	mPlayerObjectToPlayerNoMap = playerObjectToPlayerNoMap;
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(this);
	mPoints = pointsWorth;
}

FlagGameObject::~FlagGameObject() {
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Detach(this);
}

void FlagGameObject::GetFlag(int playerNo) {
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->AddItemToPlayer(InventoryBuffSystem::PlayerInventory::flag, playerNo);
#ifdef USEGL
	GetSoundObject()->TriggerSoundEvent();
#endif


	this->SetActive(false);
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->AddItemToPlayer(InventoryBuffSystem::PlayerInventory::flag, playerNo);
}

void FlagGameObject::Reset() {
	if (!this->IsActive())
	{
		this->SetActive(true);
	}
}

void NCL::CSC8503::FlagGameObject::OnPlayerInteract(int playerId) {
	if (this->IsRendered()) {
		GetFlag(playerId);
		this->SetActive(false);
	}
}


void FlagGameObject::UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved) {
	switch (invEvent) {
	case InventoryBuffSystem::flagDropped: {
		Reset();
		auto* sceneManager = SceneManager::GetSceneManager();
		if (sceneManager->IsInSingleplayer()) break;
#ifdef USEGL
		DebugNetworkedGame* networkedGame = static_cast<DebugNetworkedGame*>(sceneManager->GetCurrentScene());

		if (networkedGame->GetIsServer()) {
			networkedGame->SendInteractablePacket(this->GetNetworkObject()->GetnetworkID(), this->IsRendered(), InteractableItems::HeistItem);
		}
		else
		{
			networkedGame->GetClient()->WriteAndSendInteractablePacket(this->GetNetworkObject()->GetnetworkID(), this->IsRendered(), InteractableItems::HeistItem);
		}
		break;
#endif
	}

	default:
		break;
	}
}

const bool FlagGameObject::IsMultiplayerAndIsNotServer() {
#ifdef USEGL
	auto* sceneManager = SceneManager::GetSceneManager();
	const bool isSingleplayer = sceneManager->IsInSingleplayer();
	if (isSingleplayer)
		return false;
	DebugNetworkedGame* networkedGame = static_cast<DebugNetworkedGame*>(sceneManager->GetCurrentScene());
	const bool isServer = networkedGame->GetIsServer();
	if (!isServer)
		return true;
#endif
	return false;
}

void FlagGameObject::UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo = 0) {
	switch (buffEvent) {
#ifdef USEGL
	case BuffEvent::flagSightApplied:
		SetIsSensed(true);
		break;
	case BuffEvent::flagSightRemoved:
		SetIsSensed(false);
#endif
	default:
		break;
	}
}

void FlagGameObject::OnCollisionBegin(GameObject* otherObject) {
	if ((otherObject->GetCollisionLayer() & Player)) {
		PlayerObject* plObj = (PlayerObject*)otherObject;
		const float playerNo = plObj->GetPlayerID();

		//To fix bug where the client would get 2 flags in multiplayer
		if (mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->ItemInPlayerInventory(InventoryBuffSystem::PlayerInventory::flag, playerNo)) {
			this->SetActive(false);
			return;
		}

		if (mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->IsInventoryFull(playerNo))
			return;

		mSuspicionSystemClassPtr->GetGlobalSuspicionMetre()->SetMinGlobalSusMetre(GlobalSuspicionMetre::flagCaptured);
		plObj->AddPlayerPoints(mPoints);

		GetFlag(playerNo);
	}
}