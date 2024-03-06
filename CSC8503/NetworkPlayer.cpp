#ifdef USEGL
#include "NetworkPlayer.h"

#include "DebugNetworkedGame.h"
#include "GameClient.h"
#include "Interactable.h"
#include "NetworkedGame.h"
#include "NetworkObject.h"
#include "PhysicsObject.h"
#include "InventoryBuffSystem/Item.h"
#include "SoundManager.h"

using namespace NCL;
using namespace CSC8503;

namespace {
	constexpr int MOVE_FORWARD_INDEX = 0;
	constexpr int MOVE_LEFT_INDEX = 1;
	constexpr int MOVE_BACKWARDS_INDEX = 2;
	constexpr int MOVE_RIGHT_INDEX = 3;

	constexpr bool DEBUG_MODE = true;
}

NetworkPlayer::NetworkPlayer(NetworkedGame* game, int num) : 
	PlayerObject(game->GetLevelManager()->GetGameWorld(), LevelManager::GetLevelManager()->GetInventoryBuffSystem(),
	LevelManager::GetLevelManager()->GetSuspicionSystem(), LevelManager::GetLevelManager()->GetUiSystem(), new SoundObject(LevelManager::GetLevelManager()->GetSoundManager()->AddWalkSound()), "") {
	//this->game = game;
	mPlayerID = num;
}

NetworkPlayer::NetworkPlayer(DebugNetworkedGame* game, int num, const std::string& objName) : PlayerObject(game->GetLevelManager()->GetGameWorld(),
	LevelManager::GetLevelManager()->GetInventoryBuffSystem(),LevelManager::GetLevelManager()->GetSuspicionSystem(),
	LevelManager::GetLevelManager()->GetUiSystem(), new SoundObject(LevelManager::GetLevelManager()->GetSoundManager()->AddWalkSound()), "") {
	this->game = game;
	mPlayerID = num;
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

		if (game->GetIsServer()) {
			RayCastFromPlayer(mGameWorld, dt);
		}
		ResetPlayerInput();

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

	if (DEBUG_MODE)
	{
		PlayerObject::ShowDebugInfo(dt);
	}
}

void NetworkPlayer::MovePlayer(float dt) {
	bool isServer = game->GetIsServer();

	if (mIsLocalPlayer) {
		const Vector3 playerPos = mTransform.GetPosition();

		Debug::Print("Player Position: " + std::to_string(playerPos.x) + ", " + std::to_string(playerPos.y) + ", " + std::to_string(playerPos.z), Vector2(5, 30), Debug::MAGENTA);

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
		if (Window::GetKeyboard()->KeyDown(KeyCodes::E))
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

		mPlayerInputs.cameraYaw = game->GetLevelManager()->GetGameWorld()->GetMainCamera().GetYaw();
	}

	if (isServer == false && mIsLocalPlayer) {
		//TODO(eren.degirmenci): is dynamic casting here is bad ?
		const Vector3 fwdAxis = mGameWorld->GetMainCamera().GetForwardVector();
		const Vector3 rightAxis = mGameWorld->GetMainCamera().GetRightVector();
		mPlayerInputs.fwdAxis = fwdAxis;
		mPlayerInputs.rightAxis = rightAxis;
		game->GetClient()->WriteAndSendClientInputPacket(game->GetClientLastFullID(), mPlayerInputs);
	}
	else {
		HandleMovement(dt, mPlayerInputs);
		mIsClientInputReceived = false;
	}
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
		mPhysicsObject->AddForce(rightAxis * mMovementSpeed);

	if (playerInputs.movementButtons[MOVE_BACKWARDS_INDEX])
		mPhysicsObject->AddForce(fwdAxis * mMovementSpeed);

	if (playerInputs.movementButtons[MOVE_RIGHT_INDEX])
		mPhysicsObject->AddForce(rightAxis * mMovementSpeed);

	ActivateSprint(playerInputs.isSprinting);
	ToggleCrouch(playerInputs.isCrouching);

	StopSliding();
}


void NetworkPlayer::RayCastFromPlayer(GameWorld* world, float dt) {
	bool isRaycastTriggered = false;
	NCL::CSC8503::InteractType interactType;

	bool isThereAnyInputFromUser = (Window::GetKeyboard()->KeyPressed(KeyCodes::E) && mIsLocalPlayer) || (mPlayerInputs.isInteractButtonPressed && !mIsLocalPlayer);
	bool isPlayerHoldingInteract = (mIsLocalPlayer && Window::GetKeyboard()->KeyHeld(KeyCodes::E)) || (!mIsLocalPlayer && mPlayerInputs.isHoldingInteractButton);

	if (isThereAnyInputFromUser) {
		isRaycastTriggered = true;
		interactType = NCL::CSC8503::InteractType::Use;
		mInteractHeldDt = 0;
	}
	else if (isPlayerHoldingInteract) {
		mInteractHeldDt += dt;
		//TODO(erendgrmnc): add config or get from entity for long interact duration.
		if (mInteractHeldDt >= 5.f) {
			isRaycastTriggered = true;
			interactType = NCL::CSC8503::InteractType::LongUse;
		}
	}

	bool isEquippedItemUsed = (Window::GetMouse()->ButtonPressed(MouseButtons::Left) && GetEquippedItem() != PlayerInventory::item::none && mIsLocalPlayer) || (mPlayerInputs.isEquippedItemUsed && GetEquippedItem() != PlayerInventory::item::none);

	if (isEquippedItemUsed) {
		isRaycastTriggered = true;
		interactType = NCL::CSC8503::InteractType::ItemUse;
	}

	if (isRaycastTriggered) {
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

				if (distance > 10.f) {
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
				if (interactablePtr != nullptr && interactablePtr->CanBeInteractedWith(interactType)) {
					interactablePtr->Interact(interactType, this);
					if (interactType == ItemUse) {
						mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->UseItemInPlayerSlot(mPlayerID, mActiveItemSlot);
					}

					return;
				}

				std::cout << "Object hit " << objectHit->GetName() << std::endl;
			}
		}
	}
}

void NetworkPlayer::ControlInventory() {
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

	PlayerInventory::item equippedItem = GetEquippedItem();

	if (Window::GetMouse()->ButtonPressed(MouseButtons::Left)) {

		ItemUseType equippedItemUseType = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemUseType(equippedItem);
		mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->UseItemInPlayerSlot(mPlayerID, mActiveItemSlot);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->DropItemFromPlayer(mPlayerID, mActiveItemSlot);
	}
	if (mIsLocalPlayer) {
		//Handle Equipped Item Log
		const std::string& itemName = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemName(equippedItem);
		Debug::Print(itemName, Vector2(10, 80));
	}
}
#endif