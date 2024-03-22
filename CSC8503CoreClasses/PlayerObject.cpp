#include "GameObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "PlayerObject.h"
#include "CapsuleVolume.h"
#include "../CSC8503/InventoryBuffSystem/Item.h"
#include "Interactable.h"
#include "../CSC8503/LevelManager.h"

#include "Window.h"
#include "GameWorld.h"
#include "UISystem.h"
#include "Vent.h"
#include "Door.h"
#include "Debug.h"
#include "../CSC8503/SceneManager.h"



using namespace NCL::CSC8503;

namespace {
	constexpr float STOPPING_SPEED = 3.f;

	constexpr float CHAR_STANDING_HEIGHT = 1.4f;
	constexpr float CHAR_CROUCH_HEIGHT = 1.f;
	constexpr float CROUCH_OFFSET = 1.f;

	constexpr int MAX_CROUCH_SPEED = 5;
	constexpr int MAX_WALK_SPEED = 9;
	constexpr int MAX_SPRINT_SPEED = 20;

	constexpr float SPED_UP_MAX_CROUCH_SPEED = 7.5;
	constexpr float SPED_UP_MAX_WALK_SPEED = 13.5;
	constexpr int SPED_UP_MAX_SPRINT_SPEED = 30;

	constexpr int DEFAULT_CROUCH_SPEED = 35;
	constexpr int DEFAULT_WALK_SPEED = 40;
	constexpr int DEFAULT_SPRINT_SPEED = 50;

	//75% * Default speed, Rounded up
	constexpr int SLOWED_CROUCH_SPEED = 26;
	constexpr int SLOWED_WALK_SPEED = 30;
	constexpr int SLOWED_SPRINT_SPEED = 38;

	//150% * Default speed, Rounded up
	constexpr int SPED_UP_CROUCH_SPEED = 53;
	constexpr int SPED_UP_WALK_SPEED = 60;
	constexpr int SPED_UP_SPRINT_SPEED = 75;

	constexpr float WALK_ACCELERATING_SPEED = 1000.0f;
	constexpr float SPRINT_ACCELERATING_SPEED = 2000.0f;

	//150% * Default speed, Rounded up
	constexpr float SPED_UP_WALK_ACCELERATING_SPEED = 1500.0f;
	constexpr float SPED_UP_SPRINT_ACCELERATING_SPEED = 3000.0f;

	constexpr float LONG_INTERACT_WINDOW = 0.1f;
	constexpr float TIME_UNTIL_LONG_INTERACT = 1.5f;
	constexpr float TIME_UNTIL_PICKPOCKET = 0.75f;

	constexpr float MAX_PICKPOCKET_PITCH_DIFF = 45;

	constexpr bool DEBUG_MODE = false;
}

PlayerObject::PlayerObject(GameWorld* world, InventoryBuffSystem::InventoryBuffSystemClass* inventoryBuffSystemClassPtr,
	SuspicionSystem::SuspicionSystemClass* suspicionSystemClassPtr,
	UISystem* UI, SoundObject* soundObject,
	const std::string& objName,
	int playerID,int walkSpeed, int sprintSpeed, int crouchSpeed, Vector3 boundingVolumeOffset) {
	mName = objName;
	mGameWorld = world;
	mInventoryBuffSystemClassPtr = inventoryBuffSystemClassPtr;
	mSuspicionSystemClassPtr = suspicionSystemClassPtr;
	SetUIObject(UI);
	SetSoundObject(soundObject);
	mWalkSpeed = walkSpeed;
	mSprintSpeed = sprintSpeed;
	mCrouchSpeed = crouchSpeed;
	mMovementSpeed = walkSpeed;
	mPlayerSpeedState = Default;
	mIsCrouched = false;
	mActiveItemSlot = 0;

	mPlayerID = playerID;
	mPlayerPoints = 0;
	mIsPlayer = true;
	mHasSilentSprintBuff = false;
	mInteractHeldDt = 0;
	mAnnouncementMap.clear();
}

PlayerObject::PlayerObject(GameWorld* world, InventoryBuffSystem::InventoryBuffSystemClass* inventoryBuffSystemClassPtr,
	SuspicionSystem::SuspicionSystemClass* suspicionSystemClassPtr,
	UISystem* UI,
	const std::string& objName,
	int playerID, int walkSpeed, int sprintSpeed, int crouchSpeed, Vector3 boundingVolumeOffset) {
	mName = objName;
	mGameWorld = world;
	mInventoryBuffSystemClassPtr = inventoryBuffSystemClassPtr;
	mSuspicionSystemClassPtr = suspicionSystemClassPtr;
	SetUIObject(UI);
	mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->Attach(this);
	mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->Attach(this);
	mWalkSpeed = walkSpeed;
	mSprintSpeed = sprintSpeed;
	mCrouchSpeed = crouchSpeed;
	mMovementSpeed = walkSpeed;
	mObjectState = Walk;
	mPlayerSpeedState = Default;
	mIsCrouched = false;
	mActiveItemSlot = 0;

	mPlayerID = playerID;
	mPlayerPoints = 0;
	mIsPlayer = true;
	mHasSilentSprintBuff = false;
	mInteractHeldDt = 0;
}

PlayerObject::~PlayerObject() {
}


void PlayerObject::UpdateObject(float dt) {
	if (mPlayerSpeedState != Stunned) {
		const GameObjectState previousObjectState = mObjectState;

		MovePlayer(dt);

		NCL::CSC8503::InteractType interactType;
		if (GotRaycastInput(interactType, dt))
			RayCastFromPlayer(mGameWorld, interactType, dt);
		else
			RayCastFromPlayerForUI(mGameWorld, dt);
		if (mInventoryBuffSystemClassPtr != nullptr)
			ControlInventory();
		if (!SceneManager::GetSceneManager()->GetControllerInterface()->GetInteractHeld()) {
			mInteractHeldDt = 0;
		}

		if (previousObjectState != mObjectState)
			ChangeActiveSusCausesBasedOnState(previousObjectState, mObjectState);
	}

	AttachCameraToPlayer(mGameWorld);

	float yawValue = mGameWorld->GetMainCamera().GetYaw();
	MatchCameraRotation(yawValue);

	if (mPlayerSpeedState == SpedUp) {
		EnforceSpedUpMaxSpeeds();
	}
	else {
		EnforceMaxSpeeds();
	}


	UpdateGlobalUI(dt);
	UpdateLocalUI(dt);
	if (mIsDebugUIEnabled)
	{
		ShowDebugInfo(dt);
	}
}

void PlayerObject::ShowDebugInfo(float dt) {

	if (mHasSilentSprintBuff)
		Debug::Print("HasSilentSprint", Vector2(55, 75));
	switch (mPlayerSpeedState) {
	case SpedUp:
		Debug::Print("Sped Up", Vector2(55, 75));
		break;
	case SlowedDown:
		Debug::Print("Slowed Down", Vector2(55, 75));
		break;
	case Stunned:
		Debug::Print("Stunned", Vector2(55, 75));
		break;
	}

}

void PlayerObject::ChangeActiveSusCausesBasedOnState(const GameObjectState& previousState, const GameObjectState& currentState) {
	switch (previousState) {
	case Walk:
		mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->RemoveActiveLocalSusCause(LocalSuspicionMetre::playerWalk, mPlayerID);
		break;
	case Sprint:
		mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->RemoveActiveLocalSusCause(LocalSuspicionMetre::playerSprint, mPlayerID);
		break;
	default:
		break;
	}

	switch (currentState) {
	case Walk:
		mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->AddActiveLocalSusCause(LocalSuspicionMetre::playerWalk, mPlayerID);
		break;
	case Sprint:
		if (mHasSilentSprintBuff)
			break;
		mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->AddActiveLocalSusCause(LocalSuspicionMetre::playerSprint, mPlayerID);
		break;
	default:
		break;
	}
}

void PlayerObject::UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo) {
	if (mPlayerID != playerNo)
		return;

	switch (buffEvent) {
	case slowApplied:
		ChangeToSlowedSpeeds();
		mUi->ChangeBuffSlotTransparency(SPEED_BUFF_SLOT, 0.3);
		mUi->ChangeBuffSlotTransparency(SLOW_BUFF_SLOT, 1.0);
		break;
	case slowRemoved:
		ChangeToDefaultSpeeds();
		mUi->ChangeBuffSlotTransparency(SLOW_BUFF_SLOT, 0.3);
		break;
	case speedApplied:
		ChangeToSpedUpSpeeds();
		mUi->ChangeBuffSlotTransparency(SLOW_BUFF_SLOT, 0.3);
		mUi->ChangeBuffSlotTransparency(SPEED_BUFF_SLOT, 1.0);
		break;
	case speedRemoved:
		ChangeToDefaultSpeeds();
		mUi->ChangeBuffSlotTransparency(SPEED_BUFF_SLOT, 0.3);
		break;
	case stunApplied:
		ChangeToStunned();
		mUi->ChangeBuffSlotTransparency(STUN_BUFF_SLOT, 1.0);
		break;
	case stunRemoved:
		ChangeToDefaultSpeeds();
		mUi->ChangeBuffSlotTransparency(STUN_BUFF_SLOT, 0.3);
		break;
	case silentSprintApplied:
		mHasSilentSprintBuff = true;
		mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->
			RemoveActiveLocalSusCause(SuspicionSystem::LocalSuspicionMetre::playerSprint, mPlayerID);
		mUi->ChangeBuffSlotTransparency(SILENT_BUFF_SLOT, 1.0);
#ifdef USEGL
		this->GetSoundObject()->CloseDoorTriggered();
#endif
		break;
	case silentSprintRemoved:
		mHasSilentSprintBuff = false;
		mUi->ChangeBuffSlotTransparency(SILENT_BUFF_SLOT, 0.3);
#ifdef USEGL
		this->GetSoundObject()->CloseDoorFinished();
#endif
		mObjectState = Idle;

		break;
	default:
		break;
	}
}

PlayerInventory::item NCL::CSC8503::PlayerObject::GetEquippedItem() {
	return mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemInInventorySlot(mPlayerID, mActiveItemSlot);
}

void PlayerObject::AttachCameraToPlayer(GameWorld* world) {
	Vector3 offset = GetTransform().GetPosition();
	offset.y += 5;
	world->GetMainCamera().SetPosition(offset);
}

void PlayerObject::MovePlayer(float dt) {
	Vector3 fwdAxis = mGameWorld->GetMainCamera().GetForwardVector();
	Vector3 rightAxis = mGameWorld->GetMainCamera().GetRightVector();
	bool isIdle = true;
	if (SceneManager::GetSceneManager()->GetControllerInterface()->MoveForward()){
		Vector3 force = fwdAxis * mMovementSpeed;
		mPhysicsObject->AddForce(fwdAxis * mMovementSpeed);
		isIdle = false;
	}

	if (SceneManager::GetSceneManager()->GetControllerInterface()->MoveBackwards()){
#ifdef USEGL
		fwdAxis = fwdAxis * -1;
#endif
		mPhysicsObject->AddForce(fwdAxis * mMovementSpeed);
		isIdle = false;
	}

	if (SceneManager::GetSceneManager()->GetControllerInterface()->MoveRight()){
		mPhysicsObject->AddForce(rightAxis * mMovementSpeed);
		isIdle = false;
	}

	if (SceneManager::GetSceneManager()->GetControllerInterface()->MoveLeft()){
#ifdef USEGL
		rightAxis = rightAxis * -1;
#endif
		mPhysicsObject->AddForce(rightAxis * mMovementSpeed);
		isIdle = false;
	}

	bool isSprinting = SceneManager::GetSceneManager()->GetControllerInterface()->GetSprintDown();
	bool isCrouching = SceneManager::GetSceneManager()->GetControllerInterface()->GetCrouchPressed();
	
	GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(mGameWorld->GetMainCamera().GetPitch(), mGameWorld->GetMainCamera().GetYaw(), 0));

	if (isIdle) {
		if (mIsCrouched)
			SetObjectState(IdleCrouch);
		else
			SetObjectState(Idle);
	}
	else {
		ActivateSprint(isSprinting);
		if (mIsCrouched)
			SetObjectState(Crouch);
	}

	ToggleCrouch(isCrouching);

	StopSliding();

	if(Window::GetKeyboard()->KeyPressed(KeyCodes::F6))
		mIsDebugUIEnabled = !mIsDebugUIEnabled;
}

bool NCL::CSC8503::PlayerObject::GotRaycastInput(NCL::CSC8503::InteractType& interactType, float dt) {
	//TODO(erendgrmnc): not a best way to handle, need to refactor here later.
	if (SceneManager::GetSceneManager()->GetControllerInterface()->GetInteractPressed()) {
		interactType = NCL::CSC8503::InteractType::Use;
		mInteractHeldDt = 0;
		return true;
	}
	else if (SceneManager::GetSceneManager()->GetControllerInterface()->GetInteractHeld()) {
		mInteractHeldDt += dt;
		if (mIsDebugUIEnabled)
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
	if (SceneManager::GetSceneManager()->GetControllerInterface()->UseItemPressed() && GetEquippedItem() != PlayerInventory::item::none) {
		ItemUseType equippedItemUseType = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemUseType(GetEquippedItem());
		if (equippedItemUseType != ItemUseType::NeedInteractableToUse)
			return false;
		interactType = NCL::CSC8503::InteractType::ItemUse;
		return true;
	}
	return false;
}

void PlayerObject::RayCastFromPlayer(GameWorld* world, const NCL::CSC8503::InteractType& interactType, const float dt) {
	std::cout << "Ray fired" << std::endl;
	Ray ray = CollisionDetection::BuidRayFromCenterOfTheCamera(world->GetMainCamera());
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
				GameObject* otherPlayerObject = dynamic_cast<GameObject*>(objectHit);
				if (otherPlayerObject != nullptr && IsSeenByGameObject(otherPlayerObject)) {
					mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->
						TransferItemBetweenInventories(1,
							0, this->GetPlayerID());
				}
			}
			std::cout << "Object hit " << objectHit->GetName() << std::endl;
		}
	}
};

void PlayerObject::RayCastFromPlayerForUI(GameWorld* world, const float dt) {
	Ray ray = CollisionDetection::BuidRayFromCenterOfTheCamera(world->GetMainCamera());
	RayCollision closestCollision;

	if (world->Raycast(ray, closestCollision, true, this)) {
		auto* objectHit = (GameObject*)closestCollision.node;

		Vector2 objPos = { objectHit->GetTransform().GetPosition().x, objectHit->GetTransform().GetPosition().z };
		Vector2 playerPos = { GetTransform().GetPosition().x ,GetTransform().GetPosition().z };
		float distance = (objPos - playerPos).Length();
		RayCastIcon(objectHit, distance);

	}
	else {
		ResetRayCastIcon();
	}
}

void PlayerObject::ControlInventory() {
	if (SceneManager::GetSceneManager()->GetControllerInterface()->CheckSwitchItemOne()) {
		mActiveItemSlot = 0;
		mUi->ChangeBuffSlotTransparency(FIRST_ITEM_SLOT, 1.0);
		mUi->ChangeBuffSlotTransparency(SECOND_ITEM_SLOT, 0.5);
	}
	if (SceneManager::GetSceneManager()->GetControllerInterface()->CheckSwitchItemTwo()) {
		mActiveItemSlot = 1;
	}
	if (Window::GetMouse()->GetWheelMovement() > 0) {
		mActiveItemSlot = (mActiveItemSlot + 1 < InventoryBuffSystem::MAX_INVENTORY_SLOTS)
			? mActiveItemSlot + 1 : 0;
	}
	if (Window::GetMouse()->GetWheelMovement() < 0) {
		mActiveItemSlot = (mActiveItemSlot > 0)
			? mActiveItemSlot - 1 : InventoryBuffSystem::MAX_INVENTORY_SLOTS - 1;
	}

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

	if (SceneManager::GetSceneManager()->GetControllerInterface()->UseItemPressed()) {
		ItemUseType equippedItemUseType = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemUseType(equippedItem);
		if (equippedItemUseType == DirectUse) {
			mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->UseItemInPlayerSlot(mPlayerID, mActiveItemSlot);
		}
	}

	if (SceneManager::GetSceneManager()->GetControllerInterface()->GetDropItemPressed()) {
		mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->DropItemFromPlayer(mPlayerID, mActiveItemSlot);
	}

	const InventoryBuffSystem::PlayerInventory::item item = GetEquippedItem();
	const int usesLeft = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemUsesLeft(mPlayerID, mActiveItemSlot);

	if (item == InventoryBuffSystem::PlayerInventory::doorKey) {
		mUi->GetIcons()[mActiveItemSlot]->mTexture = mUi->GetKeyTexVec()[usesLeft-1];
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && DEBUG_MODE) {
		mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->TransferItemBetweenInventories(mPlayerID, mActiveItemSlot, 1);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F) &&
		DEBUG_MODE) {
		mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->AddItemToPlayer(PlayerInventory::flag, mPlayerID);
		mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->ApplyBuffToPlayer(PlayerBuffs::speed, mPlayerID);
	}


	if (Window::GetKeyboard()->KeyPressed(KeyCodes::V) &&
		DEBUG_MODE) {
		mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->AddItemToPlayer(PlayerInventory::soundEmitter, mPlayerID);
		mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->ApplyBuffToPlayer(PlayerBuffs::speed, mPlayerID);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM9) &&
		DEBUG_MODE) {
		mSuspicionSystemClassPtr->GetLocationBasedSuspicion()->SetMinLocationSusAmount(GetTransform().GetPosition(),20);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM0) &&
		DEBUG_MODE) {
		mSuspicionSystemClassPtr->GetLocationBasedSuspicion()->RemoveSusLocation(GetTransform().GetPosition());
	}
}

void PlayerObject::ToggleCrouch(bool crouchToggled) {
	if (crouchToggled && mObjectState == Crouch) {
		//Crouch -> Walk
		StartWalking();
	}
	else if (crouchToggled && mObjectState == Walk) {
		//Walk -> Crouch
		StartCrouching();
	}
	else if (crouchToggled && mObjectState == IdleCrouch) {
		//Crouch -> Idle
		ChangeCharacterSize(CHAR_STANDING_HEIGHT);
		mIsCrouched = false;
	}
	else if (crouchToggled && mObjectState == Idle) {
		//Idle -> Crouch
		StartCrouching();
	}
}

void PlayerObject::ActivateSprint(bool isSprinting) {
	if (isSprinting) {
		//Sprint->Sprint 
		StartSprinting();

	}
	else if (!mIsCrouched) {
		//Sprint->Walk
		StartWalking();
	}
}

void PlayerObject::StartWalking() {
	if (!(mObjectState == Walk)) {
		if (mObjectState == Crouch)
			if (mPlayerSpeedState == SpedUp)
				mMovementSpeed = SPED_UP_WALK_ACCELERATING_SPEED;
			else
				mMovementSpeed = WALK_ACCELERATING_SPEED;

		SetObjectState(Walk);
		mIsCrouched = false;
		ChangeCharacterSize(CHAR_STANDING_HEIGHT);
	}
	else
		mMovementSpeed = mWalkSpeed;
}

void PlayerObject::StartSprinting() {
	if (!(mObjectState == Sprint)) {
		if (mPlayerSpeedState == SpedUp)
			mMovementSpeed = SPED_UP_SPRINT_ACCELERATING_SPEED;
		else
			mMovementSpeed = SPRINT_ACCELERATING_SPEED;

		SetObjectState(Sprint);
		mIsCrouched = false;

		ChangeCharacterSize(CHAR_STANDING_HEIGHT);
	}
	else
		mMovementSpeed = mSprintSpeed;
}

void PlayerObject::StartCrouching() {
	if (!(mObjectState == Crouch)) {
		if (mObjectState == Walk)
			SetObjectState(Crouch);
		if (mObjectState == Idle)
			SetObjectState(IdleCrouch);

		mIsCrouched = true;
		mMovementSpeed = mCrouchSpeed;

		ChangeCharacterSize(CHAR_CROUCH_HEIGHT);

		Vector3 temp = GetTransform().GetPosition();
		temp.y -= CROUCH_OFFSET;
		GetTransform().SetPosition(temp);
	}
}

void PlayerObject::ChangeCharacterSize(float newSize) {
	static_cast<CapsuleVolume*>(mBoundingVolume)->SetHalfHeight(newSize);
}

void PlayerObject::EnforceMaxSpeeds() {
	Vector3 velocityDirection = mPhysicsObject->GetLinearVelocity();
	velocityDirection.Normalise();

	switch (mObjectState) {
	case(Crouch):
		if (mPhysicsObject->GetLinearVelocity().Length() > MAX_CROUCH_SPEED)
			mPhysicsObject->SetLinearVelocity(velocityDirection * MAX_CROUCH_SPEED);
		break;
	case(Walk):
		if (mPhysicsObject->GetLinearVelocity().Length() > MAX_WALK_SPEED)
			mPhysicsObject->SetLinearVelocity(velocityDirection * MAX_WALK_SPEED);
		break;
	case(Sprint):
		if (mPhysicsObject->GetLinearVelocity().Length() > MAX_SPRINT_SPEED)
			mPhysicsObject->SetLinearVelocity(velocityDirection * MAX_SPRINT_SPEED);
		break;
	}
}

void PlayerObject::ChangeTransparency(bool isUp, float& transparency)
{
	if (isUp == true && transparency < 1) {
		transparency = transparency + 0.05;
	}
	if (isUp == false && transparency > 0) {
		transparency = transparency - 0.05;
	}
}

void PlayerObject::RayCastIcon(GameObject* objectHit, float distance)
{
	//Open Door
	if ((objectHit->GetName() == "InteractableDoor") && (distance < 15)) {
		auto* doorHit = (Door*)objectHit;
		if (!doorHit->GetIsOpen() && !doorHit->GetIsLock()) {
			ChangeTransparency(true, mTransparencyRight);
			mUi->ChangeBuffSlotTransparency(NOTICERIGHT, mTransparencyRight);
		}
		else {
			ChangeTransparency(false, mTransparencyRight);
			mUi->ChangeBuffSlotTransparency(NOTICERIGHT, mTransparencyRight);
		}
	}
	else {
		ChangeTransparency(false, mTransparencyRight);
		mUi->ChangeBuffSlotTransparency(NOTICERIGHT, mTransparencyRight);
	}
	//Close Door
	if ((objectHit->GetName() == "InteractableDoor") && (distance < 15)) {
		auto* doorHit = (Door*)objectHit;
		if (doorHit->GetIsOpen()) {
			ChangeTransparency(true, mTransparencyLeft);
			mUi->ChangeBuffSlotTransparency(NOTICELEFT, mTransparencyLeft);
		}
		else {
			ChangeTransparency(false, mTransparencyLeft);
			mUi->ChangeBuffSlotTransparency(NOTICELEFT, mTransparencyLeft);
		}
	}
	else if (mTransparencyLeft > 0) {
		ChangeTransparency(false, mTransparencyLeft);
		mUi->ChangeBuffSlotTransparency(NOTICELEFT, mTransparencyLeft);
	}
	//Lock Door
	if ((objectHit->GetName() == "InteractableDoor") && (distance < 15) && (GetEquippedItem() == PlayerInventory::item::doorKey)) {
		auto* doorHit = (Door*)objectHit;
		if (!doorHit->GetIsOpen() && !doorHit->GetIsLock()) {
			ChangeTransparency(true, mTransparencyTop);
			mUi->ChangeBuffSlotTransparency(NOTICETOP, mTransparencyTop);
		}
		else {
			ChangeTransparency(false, mTransparencyTop);
			mUi->ChangeBuffSlotTransparency(NOTICETOP, mTransparencyTop);
		}
	}
	else {
		ChangeTransparency(false, mTransparencyTop);
		mUi->ChangeBuffSlotTransparency(NOTICETOP, mTransparencyTop);
	}
  
	//Unlock Door
	if ((objectHit->GetName() == "InteractableDoor") && (distance < 15) && (GetEquippedItem() == PlayerInventory::item::doorKey)) {

		auto* doorHit = (Door*)objectHit;
		if (!doorHit->GetIsOpen() && doorHit->GetIsLock()) {
			ChangeTransparency(true, mTransparencyBot);
			mUi->ChangeBuffSlotTransparency(NOTICEBOT, mTransparencyBot);
		}
		else {
			ChangeTransparency(false, mTransparencyBot);
			mUi->ChangeBuffSlotTransparency(NOTICEBOT, mTransparencyBot);
		}
	}
	else {
		ChangeTransparency(false, mTransparencyBot);
		mUi->ChangeBuffSlotTransparency(NOTICEBOT, mTransparencyBot);
	}

	if ((objectHit->GetName() == "InteractableDoor") && (distance < 15)) {
		auto* doorHit = (Door*)objectHit;
		if (!doorHit->GetIsOpen() && doorHit->GetIsLock()) {
			ChangeTransparency(true, mTransparencyTopRight);
			mUi->ChangeBuffSlotTransparency(NOTICETOPRIGHT, mTransparencyTopRight);
		}
		else {
			ChangeTransparency(false, mTransparencyTopRight);
			mUi->ChangeBuffSlotTransparency(NOTICETOPRIGHT, mTransparencyTopRight);
		}
	}
	else {
		ChangeTransparency(false, mTransparencyTopRight);
		mUi->ChangeBuffSlotTransparency(NOTICETOPRIGHT, mTransparencyTopRight);
	}

	//Use ScrewDriver
	if ((objectHit->GetName() == "Vent") && (distance < 15) && (GetEquippedItem() == PlayerInventory::item::screwdriver)) {
		auto* ventHit = (Vent*)objectHit;
		if (!ventHit->IsOpen()) {
			ChangeTransparency(true, mTransparencyBotRight);
			mUi->ChangeBuffSlotTransparency(NOTICEBOTRIGHT, mTransparencyBotRight);
		}
		else {
			ChangeTransparency(false, mTransparencyBotRight);
			mUi->ChangeBuffSlotTransparency(NOTICEBOTRIGHT, mTransparencyBotRight);
		}
	}
	else {
		ChangeTransparency(false, mTransparencyBotRight);
		mUi->ChangeBuffSlotTransparency(NOTICEBOTRIGHT, mTransparencyBotRight);
	}


	//Use Vent
	if ((objectHit->GetName() == "Vent") && (distance < 15)) {
		auto* ventHit = (Vent*)objectHit;
		if (ventHit->IsOpen()) {
			ChangeTransparency(true, mTransparencyBotLeft);
			mUi->ChangeBuffSlotTransparency(NOTICEBOTLEFT, mTransparencyBotLeft);
		}
		else {
			ChangeTransparency(false, mTransparencyBotLeft);
			mUi->ChangeBuffSlotTransparency(NOTICEBOTLEFT, mTransparencyBotLeft);
		}
	}
	else {
		ChangeTransparency(false, mTransparencyBotLeft);
		mUi->ChangeBuffSlotTransparency(NOTICEBOTLEFT, mTransparencyBotLeft);
	}


}

void NCL::CSC8503::PlayerObject::ResetRayCastIcon()
{
	ChangeTransparency(false, mTransparencyTop);
	ChangeTransparency(false, mTransparencyBot);
	ChangeTransparency(false, mTransparencyLeft);
	ChangeTransparency(false, mTransparencyRight);
	ChangeTransparency(false, mTransparencyBotLeft);

	mUi->ChangeBuffSlotTransparency(NOTICETOP, mTransparencyTop);
	mUi->ChangeBuffSlotTransparency(NOTICEBOT, mTransparencyBot);
	mUi->ChangeBuffSlotTransparency(NOTICELEFT, mTransparencyLeft);
	mUi->ChangeBuffSlotTransparency(NOTICERIGHT, mTransparencyRight);
	mUi->ChangeBuffSlotTransparency(NOTICEBOTLEFT, mTransparencyBotLeft);
}

float PlayerObject::SusLinerInterpolation(float dt)
{
	if (dt * mUiTime < 0.6) {
		mUiTime++;
		tempSusValue = tempSusValue + (mSusValue - mLastSusValue) * 0.016;
		if (tempSusValue < 0.016) {
			tempSusValue = 0;
		}
	}
	else {
		mUiTime = 1;
		mLastSusValue = tempSusValue;
		mSusValue = mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->GetLocalSusMetreValue(mPlayerID);
	}
	float iconValue = 100.00 - (tempSusValue * 0.7 + 14.00);
	return iconValue;
}

bool PlayerObject::IsSeenByGameObject(GameObject* otherGameObject) {
	float thisPitch = GetTransform().GetOrientation().ToEuler().y;
	float otherPitch = otherGameObject->GetTransform().GetOrientation().ToEuler().y;

	float PitchDiff = abs(otherPitch - thisPitch);
	return PitchDiff <= MAX_PICKPOCKET_PITCH_DIFF;
}

void PlayerObject::EnforceSpedUpMaxSpeeds() {
	Vector3 velocityDirection = mPhysicsObject->GetLinearVelocity();
	velocityDirection.Normalise();

	switch (mObjectState) {
	case(Crouch):
		if (mPhysicsObject->GetLinearVelocity().Length() > SPED_UP_MAX_CROUCH_SPEED)
			mPhysicsObject->SetLinearVelocity(velocityDirection * SPED_UP_MAX_CROUCH_SPEED);
		break;
	case(Walk):
		if (mPhysicsObject->GetLinearVelocity().Length() > SPED_UP_MAX_WALK_SPEED)
			mPhysicsObject->SetLinearVelocity(velocityDirection * SPED_UP_MAX_WALK_SPEED);
		break;
	case(Sprint):
		if (mPhysicsObject->GetLinearVelocity().Length() > SPED_UP_MAX_SPRINT_SPEED)
			mPhysicsObject->SetLinearVelocity(velocityDirection * SPED_UP_MAX_SPRINT_SPEED);
		break;
	}
}

void PlayerObject::ChangeToDefaultSpeeds() {
	mCrouchSpeed = DEFAULT_CROUCH_SPEED;
	mWalkSpeed = DEFAULT_WALK_SPEED;
	mSprintSpeed = DEFAULT_SPRINT_SPEED;

	mPlayerSpeedState = Default;

	SetObjectState(Idle);

}

void PlayerObject::ChangeToSlowedSpeeds() {
	mCrouchSpeed = SLOWED_CROUCH_SPEED;
	mWalkSpeed = SLOWED_WALK_SPEED;
	mSprintSpeed = SLOWED_SPRINT_SPEED;

	mPlayerSpeedState = SlowedDown;

	SetObjectState(Idle);

}

void PlayerObject::ChangeToSpedUpSpeeds() {
	mCrouchSpeed = SPED_UP_CROUCH_SPEED;
	mWalkSpeed = SPED_UP_WALK_SPEED;
	mSprintSpeed = SPED_UP_SPRINT_SPEED;

	mPlayerSpeedState = SpedUp;

	SetObjectState(Idle);

}

void PlayerObject::ChangeToStunned() {
	mCrouchSpeed = 0;
	mWalkSpeed = 0;
	mSprintSpeed = 0;

	mPhysicsObject->SetLinearVelocity(Vector3(0,0,0));
	mPlayerSpeedState = Stunned;

	SetObjectState(Idle);

}

void NCL::CSC8503::PlayerObject::UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved) {
	switch (invEvent)
	{
	case InventoryBuffSystem::flagAdded:
		mSuspicionSystemClassPtr->GetGlobalSuspicionMetre()->SetMinGlobalSusMetre(GlobalSuspicionMetre::flagCaptured);
		AddAnnouncement(AnnouncementType::FlagAddedAnnouncement, 8, playerNo);
		break;
	case InventoryBuffSystem::flagDropped:
		AddAnnouncement(AnnouncementType::FlagDroppedAnnouncement, 8, playerNo);
		break;
	case InventoryBuffSystem::disguiseItemUsed:
		break;
	case InventoryBuffSystem::soundEmitterUsed:
		break;
	case InventoryBuffSystem::screwdriverUsed:
		break;
	case InventoryBuffSystem::stunItemUsed:
		//TODO(erendgrmnc): handle multiplayer
		//mInventoryBuffSystemClassPtr->GetPlayerBuffsPtr()->ApplyBuffToPlayer(PlayerBuffs::stun, playerNo);
		//TODO(erendgrnc): stun guards only in singleplayer
		LevelManager::GetLevelManager()->AddBuffToGuards(PlayerBuffs::stun);
		break;
	default:
		break;
	}
}

void PlayerObject::UpdateGlobalUI(float dt) {
	int announcementY = 15;
	for (auto& entry : mAnnouncementMap) {
		if (entry.second > 0) {
			entry.second -= dt;
			Debug::Print(entry.first, Vector2(0, announcementY));
			announcementY += 5;
		}
	}
}



void PlayerObject::UpdateLocalUI(float dt) {
	//SusBar
	float iconValue = SusLinerInterpolation(dt);
	mUi->GetIcons()[SUSPISION_BAR_SLOT]->mTexture = mUi->GetSusBarTexVec()[0];
	if (mSusValue > 33) {
		mUi->GetIcons()[SUSPISION_BAR_SLOT]->mTexture = mUi->GetSusBarTexVec()[1];
		if (mSusValue > 66) {
			mUi->GetIcons()[SUSPISION_BAR_SLOT]->mTexture = mUi->GetSusBarTexVec()[2];
		}
	}
	mUi->SetIconPosition(Vector2(90.00, iconValue), *mUi->GetIcons()[SUSPISION_INDICATOR_SLOT]);

	Debug::Print("POINTS: " + to_string(int(mPlayerPoints)), Vector2(0, 6));
	Debug::Print("Sus lvl:", Vector2(80, 95));
	Debug::Print(std::to_string((int)mSuspicionSystemClassPtr->GetLocalSuspicionMetre()->GetLocalSusMetreValue(mPlayerID)), Vector2(95, 95));

	//global suspicion
	float globalSusValue = mSuspicionSystemClassPtr->GetGlobalSuspicionMetre()->GetGlobalSusMeter();
	if (globalSusValue > 33) {
		mUi->ChangeBuffSlotTransparency(ALARM, abs(sin(mAlarmTime) * 0.5));
		mAlarmTime = mAlarmTime + dt;
	}
	if (globalSusValue < 33 && mUi->GetIcons()[ALARM]->mTransparency>0) {
		mUi->GetIcons()[ALARM]->mTransparency = mUi->GetIcons()[ALARM]->mTransparency - dt;
		mAlarmTime = 0;
	}
	//Handle Equipped Item Log
	const PlayerInventory::item equippedItem = GetEquippedItem();
	std::string& itemName = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemName(equippedItem);

	Debug::Print(itemName, Vector2(40, 82));
	const int usesLeft = mInventoryBuffSystemClassPtr->GetPlayerInventoryPtr()->GetItemUsesLeft(mPlayerID, mActiveItemSlot);

	if(usesLeft>1){
		const std::string& usesLeftStr = "UsesLeft : " + to_string(usesLeft);
		Debug::Print(usesLeftStr, Vector2(39, 87));
	}

	Debug::Print(" Alert lvl:", Vector2(74, 98));
	Debug::Print(std::to_string((int)mSuspicionSystemClassPtr->GetGlobalSuspicionMetre()->GetGlobalSusMeter()), Vector2(95, 98));
}

void PlayerObject::MatchCameraRotation(float yawValue) {
	Matrix4 yawRotation = Matrix4::Rotation(yawValue, Vector3(0, 1, 0));
	GetTransform().SetOrientation(yawRotation);
}

void PlayerObject::StopSliding() {
	if ((mPhysicsObject->GetLinearVelocity().Length() < STOPPING_SPEED) && (mPhysicsObject->GetForce() == Vector3(0, 0, 0))) {
		float fallingSpeed = mPhysicsObject->GetLinearVelocity().y;
		mPhysicsObject->SetLinearVelocity(Vector3(0, fallingSpeed, 0));
	}
}
