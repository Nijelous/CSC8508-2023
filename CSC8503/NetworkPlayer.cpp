#include "SceneManager.h"
#ifdef USEGL
#include "NetworkPlayer.h"

#include "DebugNetworkedGame.h"
#include "LevelManager.h"
#include "GameClient.h"
#include "Interactable.h"
#include "NetworkedGame.h"
#include "NetworkObject.h"
#include "PhysicsObject.h"
#include "InventoryBuffSystem/Item.h"
#include "SoundManager.h"
#include "PrisonDoor.h"
#include "Debug.h"

using namespace NCL;
using namespace CSC8503;

namespace {
	constexpr int MOVE_FORWARD_INDEX = 0;
	constexpr int MOVE_LEFT_INDEX = 1;
	constexpr int MOVE_BACKWARDS_INDEX = 2;
	constexpr int MOVE_RIGHT_INDEX = 3;

	constexpr float LONG_INTERACT_WINDOW = 0.1f;
	constexpr float TIME_UNTIL_LONG_INTERACT = 1.5f;
	constexpr float TIME_UNTIL_PICKPOCKET = 0.75f;

	constexpr float MAX_PICKPOCKET_PITCH_DIFF = 20;

	constexpr bool DEBUG_MODE = false;
}

NetworkPlayer::NetworkPlayer(NetworkedGame* game, int num) : 
	PlayerObject(game->GetLevelManager()->GetGameWorld(), LevelManager::GetLevelManager()->GetInventoryBuffSystem(),
	LevelManager::GetLevelManager()->GetSuspicionSystem(),new UISystem(), new SoundObject(LevelManager::GetLevelManager()->GetSoundManager()->AddWalkSound()), "") {
	//this->game = game;
	mPlayerID = num;
}

NetworkPlayer::NetworkPlayer(DebugNetworkedGame* game, int num, const std::string& objName) : PlayerObject(game->GetLevelManager()->GetGameWorld(),
	LevelManager::GetLevelManager()->GetInventoryBuffSystem(),LevelManager::GetLevelManager()->GetSuspicionSystem(),
	LevelManager::GetLevelManager()->GetUiSystem(), new SoundObject(LevelManager::GetLevelManager()->GetSoundManager()->AddWalkSound()), "") {
	this->game = game;
	mPlayerID = num;
	this->SetName(objName);

}

NetworkPlayer::~NetworkPlayer() {
}

void NetworkPlayer::OnCollisionBegin(GameObject* otherObject) {
	if (game) {
		if (dynamic_cast<NetworkPlayer*>(otherObject)) {
			game->OnPlayerCollision(this, (NetworkPlayer*)otherObject);
		}
	}
}

void NetworkPlayer::SetPlayerInput(const PlayerInputs& playerInputs) {
	mPlayerInputs = playerInputs;
	mIsClientInputReceived = true;
	mCameraYaw = playerInputs.cameraYaw;
}

void NetworkPlayer::SetIsLocalPlayer(bool isLocalPlayer) {
	mIsLocalPlayer = isLocalPlayer;
}

void NetworkPlayer::SetCameraYaw(float cameraYaw) {
	mCameraYaw = cameraYaw;
}

void NetworkPlayer::ResetPlayerInput() {
	mPlayerInputs = PlayerInputs();
}

void NetworkPlayer::UpdateObject(float dt) {
	if (mPlayerSpeedState != Stunned) {
		MovePlayer(dt);

		//if (game->GetIsServer()) 
		NCL::CSC8503::InteractType interactType;
		if (GotRaycastInput(interactType, dt))
			RayCastFromPlayer(mGameWorld, interactType, dt);
		else
			PlayerObject::RayCastFromPlayerForUI(mGameWorld,dt);

		if (mInventoryBuffSystemClassPtr != nullptr)
			ControlInventory();

		if (!Window::GetKeyboard()->KeyHeld(KeyCodes::E)) {
			mInteractHeldDt = 0;
		}
	}

	if (mIsLocalPlayer) {
		AttachCameraToPlayer(game->GetLevelManager()->GetGameWorld());
		mCameraYaw = game->GetLevelManager()->GetGameWorld()->GetMainCamera().GetYaw();
	}

	if (mIsLocalPlayer || game->GetIsServer()) {
		MatchCameraRotation(mCameraYaw);
	}

	if (mPlayerSpeedState == SpedUp)
		EnforceSpedUpMaxSpeeds();
	else
		EnforceMaxSpeeds();
	
	if (mIsLocalPlayer){
		PlayerObject::UpdateGlobalUI(dt);
		PlayerObject::UpdateLocalUI(dt);
		if (mIsDebugUIEnabled)
			PlayerObject::ShowDebugInfo(dt);
	}
}

void NetworkPlayer::MovePlayer(float dt) {
	bool isServer = game->GetIsServer();

	if (mIsLocalPlayer) {

		ResetPlayerInput();
		const Vector3 playerPos = mTransform.GetPosition();

		//Debug::Print("Player Position: " + std::to_string(playerPos.x) + ", " + std::to_string(playerPos.y) + ", " + std::to_string(playerPos.z), Vector2(5, 30), Debug::MAGENTA);

		if (Window::GetKeyboard()->KeyDown(KeyCodes::W))
			mPlayerInputs.movementButtons[MOVE_FORWARD_INDEX] = true;

		if (Window::GetKeyboard()->KeyDown(KeyCodes::A))
			mPlayerInputs.movementButtons[MOVE_LEFT_INDEX] = true;

		if (Window::GetKeyboard()->KeyDown(KeyCodes::S))
			mPlayerInputs.movementButtons[MOVE_BACKWARDS_INDEX] = true;

		if (Window::GetKeyboard()->KeyDown(KeyCodes::D))
			mPlayerInputs.movementButtons[MOVE_RIGHT_INDEX] = true;

		if (Window::GetKeyboard()->KeyDown(KeyCodes::SHIFT))
			mPlayerInputs.isSprinting = true;

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::CONTROL))
			mPlayerInputs.isCrouching = true;
		if (Window::GetMouse()->ButtonDown(MouseButtons::Left))
			mPlayerInputs.isEquippedItemUsed = true;
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::E))
			mPlayerInputs.isInteractButtonPressed = true;
		if (Window::GetKeyboard()->KeyHeld(KeyCodes::E))
			mPlayerInputs.isHoldingInteractButton = true;

		if (mPlayerInputs.isInteractButtonPressed || mPlayerInputs.isEquippedItemUsed || mPlayerInputs.isHoldingInteractButton) {
			Ray ray = CollisionDetection::BuidRayFromCenterOfTheCamera(mGameWorld->GetMainCamera());
			mPlayerInputs.rayFromPlayer = ray;
		}

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::L) &&
			DEBUG_MODE) {
			mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->ApplyBuffToPlayer(PlayerBuffs::slow, mPlayerID);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::F) &&
			DEBUG_MODE) {
			mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->ApplyBuffToPlayer(PlayerBuffs::flagSight, mPlayerID);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::N) &&
			!Window::GetKeyboard()->KeyHeld(KeyCodes::SHIFT) &&
			DEBUG_MODE) {
			mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->AddActiveLocalSusCause(LocalSuspicionMetre::guardsLOS, mPlayerID);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::M) &&
			DEBUG_MODE) {
			mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->RemoveActiveLocalSusCause(LocalSuspicionMetre::guardsLOS, mPlayerID);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::N) &&
			Window::GetKeyboard()->KeyHeld(KeyCodes::SHIFT) &&
			DEBUG_MODE) {
			mSuspicionSystemClassPtr->GetGlobalSuspicionMetre()->SetMinGlobalSusMetre(GlobalSuspicionMetre::flagCaptured);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::F) &&
			DEBUG_MODE) {
			mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->AddItemToPlayer(PlayerInventory::flag, mPlayerID);
			mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->ApplyBuffToPlayer(PlayerBuffs::speed, mPlayerID);
		}
		mPlayerInputs.cameraYaw = game->GetLevelManager()->GetGameWorld()->GetMainCamera().GetYaw();
	}

	const GameObjectState previousObjectState = mObjectState;

	if (isServer == false && mIsLocalPlayer) {
		//TODO(eren.degirmenci): is dynamic casting here is bad ?
		const Vector3 fwdAxis = mGameWorld->GetMainCamera().GetForwardVector();
		const Vector3 rightAxis = mGameWorld->GetMainCamera().GetRightVector();
		mPlayerInputs.fwdAxis = fwdAxis;
		mPlayerInputs.rightAxis = rightAxis;
		game->GetClient()->WriteAndSendClientInputPacket(game->GetClientLastFullID(), mPlayerInputs);
	}
	else if (isServer) {

		HandleMovement(dt, mPlayerInputs);

		mIsClientInputReceived = false;
	}

	ChangeActiveSusCausesBasedOnState(previousObjectState, mObjectState);
}

void NCL::CSC8503::NetworkPlayer::AddAnnouncement(AnnouncementType announcementType, float time, int playerNo){
	PlayerObject::AddAnnouncement(announcementType, time, playerNo);
	if (game->GetIsServer() )
		SendAnnouncementPacket(announcementType, time, playerNo);
	else 
	if (!game->GetIsServer())
		game->GetClient()->WriteAndSendAnnouncementSyncPacket(announcementType, time, playerNo);
}

void NetworkPlayer::SendAnnouncementPacket(AnnouncementType announcementType, float time, int playerNo){
	game->SendAnnouncementSyncPacket(announcementType,time, playerNo);
}

void NetworkPlayer::SyncAnnouncements(AnnouncementType announcementType, float time, int playerNo){
	const std::string annString = mAnnouncementTypeToStringMap[announcementType] + std::to_string(playerNo) + '!';
	mAnnouncementMap[annString] = time;
}

void NetworkPlayer::HandleMovement(float dt, const PlayerInputs& playerInputs) {
	Vector3 fwdAxis;
	Vector3 rightAxis;
	if (mIsLocalPlayer) {
		fwdAxis = mGameWorld->GetMainCamera().GetForwardVector();
		rightAxis = mGameWorld->GetMainCamera().GetRightVector();
	}
	else {
		fwdAxis = playerInputs.fwdAxis;
		rightAxis = playerInputs.rightAxis;
	}

	if (playerInputs.movementButtons[MOVE_FORWARD_INDEX])
		mPhysicsObject->AddForce(fwdAxis * mMovementSpeed);

	if (playerInputs.movementButtons[MOVE_LEFT_INDEX])
		mPhysicsObject->AddForce(-rightAxis * mMovementSpeed);

	if (playerInputs.movementButtons[MOVE_BACKWARDS_INDEX])
		mPhysicsObject->AddForce(-fwdAxis * mMovementSpeed);

	if (playerInputs.movementButtons[MOVE_RIGHT_INDEX])
		mPhysicsObject->AddForce(rightAxis * mMovementSpeed);

	bool isIdle = true;
	for (auto buttonState : mPlayerInputs.movementButtons)
		if (buttonState)
			isIdle = false;

	if (isIdle) {
		if (mIsCrouched)
			SetObjectState(IdleCrouch);
		else
			SetObjectState(Idle);
	}
	else {
		ActivateSprint(playerInputs.isSprinting);
		if (mIsCrouched)
			SetObjectState(Crouch);
	}
	ToggleCrouch(playerInputs.isCrouching);

	StopSliding();
}

bool NCL::CSC8503::NetworkPlayer::GotRaycastInput(NCL::CSC8503::InteractType& interactType, const float dt){
	bool isThereAnyInputFromUser = (Window::GetKeyboard()->KeyPressed(KeyCodes::E) && mIsLocalPlayer) || (mPlayerInputs.isInteractButtonPressed && !mIsLocalPlayer);
	bool isPlayerHoldingInteract = (Window::GetKeyboard()->KeyHeld(KeyCodes::E)) || (!mIsLocalPlayer && mPlayerInputs.isHoldingInteractButton);

	if (isThereAnyInputFromUser) {
		interactType = NCL::CSC8503::InteractType::Use;
		mInteractHeldDt = 0;
		return true;
	}
	else if (isPlayerHoldingInteract) {
		//TODO(erendgrmnc): add config or get from entity for long interact duration.
		mInteractHeldDt += dt;
		Debug::Print(to_string(mInteractHeldDt), Vector2(55, 98));
		if (mInteractHeldDt >= TIME_UNTIL_PICKPOCKET - LONG_INTERACT_WINDOW &&
			mInteractHeldDt <= TIME_UNTIL_PICKPOCKET + LONG_INTERACT_WINDOW) {
			interactType = NCL::CSC8503::InteractType::PickPocket;
			if (mIsDebugUIEnabled)
				Debug::Print("PickPocket", Vector2(55, 95));

			return true;
		}
		if (mInteractHeldDt >= TIME_UNTIL_LONG_INTERACT - LONG_INTERACT_WINDOW &&
			mInteractHeldDt <= TIME_UNTIL_LONG_INTERACT + LONG_INTERACT_WINDOW) {
			interactType = NCL::CSC8503::InteractType::LongUse;
			if (mIsDebugUIEnabled)
				Debug::Print("LongUse", Vector2(55, 95));

			return true;
		}
	}

	bool isEquippedItemUsed = (Window::GetMouse()->ButtonPressed(MouseButtons::Left) && GetEquippedItem() != PlayerInventory::item::none && mIsLocalPlayer) ||
		(mPlayerInputs.isEquippedItemUsed && GetEquippedItem() != PlayerInventory::item::none);

	if (isEquippedItemUsed) {
		ItemUseType equippedItemUseType = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemUseType(GetEquippedItem());
		if (equippedItemUseType != ItemUseType::NeedInteractableToUse)
			return false;
		interactType = NCL::CSC8503::InteractType::ItemUse;

		return true;
	}
	return false;
}


void NetworkPlayer::RayCastFromPlayer(GameWorld* world, const NCL::CSC8503::InteractType& interactType, float dt) {
	Ray ray;
	if (!mIsLocalPlayer) {
		ray = mPlayerInputs.rayFromPlayer;
	}
	else {
		ray = CollisionDetection::BuidRayFromCenterOfTheCamera(world->GetMainCamera());
	}
	std::cout << "Ray fired" << std::endl;
	RayCollision closestCollision;

	if (world->Raycast(ray, closestCollision, true, this)) {
		auto* objectHit = (GameObject*)closestCollision.node;
		if (objectHit) {
			Vector3 objPos = objectHit->GetTransform().GetPosition();

			Vector3 playerPos = GetTransform().GetPosition();

			float distance = (objPos - playerPos).Length();

			if (distance > 17.5f) {
				std::cout << "Nothing hit in range" << std::endl;
				return;
			}

			//Check if object is an item.
			Item* item = dynamic_cast<Item*>(objectHit);
			if (item != nullptr) {
				item->OnPlayerInteract(mPlayerID);
				return;
			}
					
			//Check if object is an interactable.
			Interactable* interactablePtr = dynamic_cast<Interactable*>(objectHit);
			if (interactablePtr != nullptr && interactablePtr->CanBeInteractedWith(interactType, this)) {
				interactablePtr->Interact(interactType, this);
				if (interactType == ItemUse) {
					mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->UseItemInPlayerSlot(mPlayerID, mActiveItemSlot);
				}

				return;
			}

			if (interactType == PickPocket)
			{
				NetworkPlayer* otherPlayerObject = dynamic_cast<NetworkPlayer*>(objectHit);
				if (otherPlayerObject != nullptr && IsSeenByGameObject(otherPlayerObject)) {
					mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->
						TransferItemBetweenInventories(otherPlayerObject->GetPlayerID(),
							otherPlayerObject->GetActiveItemSlot(), this->GetPlayerID());
				}
			}

			std::cout << "Object hit " << objectHit->GetName() << std::endl;
		}
	}
}

void NetworkPlayer::ControlInventory() {
	if (!mIsLocalPlayer)
		return;

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1))
		mActiveItemSlot = 0;

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2))
		mActiveItemSlot = 1;

	if (Window::GetMouse()->GetWheelMovement() > 0)
		mActiveItemSlot = (mActiveItemSlot + 1 < InventoryBuffSystem::MAX_INVENTORY_SLOTS)
		? mActiveItemSlot + 1 : 0;

	if (Window::GetMouse()->GetWheelMovement() < 0)
		mActiveItemSlot = (mActiveItemSlot > 0)
		? mActiveItemSlot - 1 : InventoryBuffSystem::MAX_INVENTORY_SLOTS - 1;

	switch (mActiveItemSlot) {
	case 0:
		mUi->ChangeBuffSlotTransparency(FIRST_ITEM_SLOT, 1.0);
		mUi->ChangeBuffSlotTransparency(SECOND_ITEM_SLOT, 0.25);
		break;
	case 1:
		mUi->ChangeBuffSlotTransparency(FIRST_ITEM_SLOT, 0.25);
		mUi->ChangeBuffSlotTransparency(SECOND_ITEM_SLOT, 1.0);
		break;
	}

	PlayerInventory::item equippedItem = GetEquippedItem();

	if (Window::GetMouse()->ButtonPressed(MouseButtons::Left)) {
		ItemUseType equippedItemUseType = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemUseType(equippedItem);
		if (equippedItemUseType == DirectUse) {
			mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->UseItemInPlayerSlot(mPlayerID, mActiveItemSlot);
		}
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->DropItemFromPlayer(mPlayerID, mActiveItemSlot);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && DEBUG_MODE) {
		mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->TransferItemBetweenInventories(mPlayerID, mActiveItemSlot, 1);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::O) && DEBUG_MODE) {
		LevelManager::GetLevelManager()->GetPrisonDoor()->SetIsOpen(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::P) && DEBUG_MODE) {
		LevelManager::GetLevelManager()->GetPrisonDoor()->SetIsOpen(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F6))
		mIsDebugUIEnabled = !mIsDebugUIEnabled;
}
#endif