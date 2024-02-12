#include "GameObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"
#include "PlayerObject.h"
#include "CapsuleVolume.h"

#include "Window.h"
#include "GameWorld.h"

using namespace NCL::CSC8503;

namespace {
	constexpr float CHAR_STANDING_HEIGHT = 1.4f;
	constexpr float CHAR_CROUCH_HEIGHT = .7f;
	constexpr float CROUCH_OFFSET = 1;

	constexpr float BRAKING_SPEED = 50.0f;
	constexpr float WALK_ACCELERATING_SPEED = 1000.0f;
	constexpr float SPRINT_ACCELERATING_SPEED = 2000.0f;
}

PlayerObject::PlayerObject(GameWorld* world, const std::string& objName, int walkSpeed, int sprintSpeed, int crouchSpeed, Vector3 boundingVolumeOffset) {
	mName = objName;
	mGameWorld = world;

	mWalkSpeed= walkSpeed;
	mSprintSpeed = sprintSpeed;
	mCrouchSpeed = crouchSpeed;
	mMovementSpeed = walkSpeed;
	mPlayerState = Walk;
	mIsCrouched = false;

	mIsPlayer = true;
}

PlayerObject::~PlayerObject() {

}

void PlayerObject::UpdateObject(float dt)
{
	MovePlayer(dt);
	AttachCameraToPlayer(mGameWorld);
	
	float yawValue = mGameWorld->GetMainCamera().GetYaw();
	MatchCameraRotation(yawValue);

	EnforceMaxSpeeds();
}

void PlayerObject::AttachCameraToPlayer(GameWorld* world) {
	Vector3 offset = GetTransform().GetPosition();
	offset.y += 3;
	world->GetMainCamera().SetPosition(offset);
}

void PlayerObject::MovePlayer(float dt) {
	Vector3 fwdAxis = mGameWorld->GetMainCamera().GetForwardVector();
	Vector3 rightAxis = mGameWorld->GetMainCamera().GetRightVector();

	if (Window::GetKeyboard()->KeyDown(KeyCodes::W))
		mPhysicsObject->AddForce(fwdAxis * mMovementSpeed);

	if (Window::GetKeyboard()->KeyDown(KeyCodes::S))
		mPhysicsObject->AddForce(fwdAxis * mMovementSpeed);

	if (Window::GetKeyboard()->KeyDown(KeyCodes::A))
		mPhysicsObject->AddForce(rightAxis * mMovementSpeed);

	if (Window::GetKeyboard()->KeyDown(KeyCodes::D))
		mPhysicsObject->AddForce(rightAxis * mMovementSpeed);

	bool isSprinting = Window::GetKeyboard()->KeyDown(KeyCodes::SHIFT);
	bool isCrouching = Window::GetKeyboard()->KeyPressed(KeyCodes::CONTROL);
	ActivateSprint(isSprinting);
	ToggleCrouch(isCrouching);

	StopSliding();
}

void PlayerObject::ToggleCrouch(bool isCrouching) {
	if (isCrouching && mPlayerState == Crouch)
		StartWalking();
	else if (isCrouching && mPlayerState == Walk)
		StartCrouching();
}

void PlayerObject::ActivateSprint(bool isSprinting) {
	if (isSprinting)
		StartSprinting();
	else if (!mIsCrouched)
		StartWalking();
	else if (mIsCrouched)
		StartCrouching();
}

void PlayerObject::StartWalking() {
	if (!(mPlayerState == Walk)) {
		if (mPlayerState == Crouch)
			mMovementSpeed = WALK_ACCELERATING_SPEED;

		mPlayerState = Walk;
		mIsCrouched = false;

		ChangeCharacterSize(CHAR_STANDING_HEIGHT);
	}
	else
		mMovementSpeed = mWalkSpeed;
}

void PlayerObject::StartSprinting() {
	if (!(mPlayerState == Sprint)) {
		mMovementSpeed = SPRINT_ACCELERATING_SPEED;

		mPlayerState = Sprint;
		mIsCrouched = false;

		ChangeCharacterSize(CHAR_STANDING_HEIGHT);
	}
	else
		mMovementSpeed = mSprintSpeed;
}

void PlayerObject::StartCrouching() {
	if (!(mPlayerState == Crouch)) {
		mPlayerState = Crouch;
		mIsCrouched = true;
		mMovementSpeed = mCrouchSpeed;

		ChangeCharacterSize(CHAR_CROUCH_HEIGHT);

		Vector3 temp = GetTransform().GetPosition();
		temp.y -= CROUCH_OFFSET;
		GetTransform().SetPosition(temp);
	}
}

void PlayerObject::ChangeCharacterSize(float newSize) {
	dynamic_cast<CapsuleVolume*>(mBoundingVolume)->SetHalfHeight(newSize);
}

void PlayerObject::EnforceMaxSpeeds() {
	Vector3 velocityDirection = mPhysicsObject->GetLinearVelocity();
	velocityDirection.Normalise();

	switch (mPlayerState) {
	case(Crouch):
		if (mPhysicsObject->GetLinearVelocity().Length() > 5) 
			mPhysicsObject->AddForce(-velocityDirection * BRAKING_SPEED);
		break;
	case(Walk):
		if (mPhysicsObject->GetLinearVelocity().Length() > 9) 
			mPhysicsObject->AddForce(-velocityDirection * BRAKING_SPEED);
		break;
	case(Sprint):
		if (mPhysicsObject->GetLinearVelocity().Length() > 20) 
			mPhysicsObject->AddForce(-velocityDirection * BRAKING_SPEED);
		break;
	
	}
}

void PlayerObject::MatchCameraRotation(float yawValue) {
	Matrix4 yawRotation = Matrix4::Rotation(yawValue, Vector3(0, 1, 0));
	GetTransform().SetOrientation(yawRotation);
}

void PlayerObject::StopSliding() {
	if ((mPhysicsObject->GetLinearVelocity().Length() < 1) && (mPhysicsObject->GetForce() == Vector3(0,0,0))) {
		float fallingSpeed = mPhysicsObject->GetLinearVelocity().y;
		mPhysicsObject->SetLinearVelocity(Vector3(0, fallingSpeed, 0));
	}
}

