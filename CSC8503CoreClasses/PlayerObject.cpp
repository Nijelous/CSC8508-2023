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
	MatchCameraRotation();
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

	bool isSprinting = Window::GetKeyboard()->KeyPressed(KeyCodes::SHIFT);
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
		std::cout << "Walking" << std::endl;
		mPlayerState = Walk;
		mIsCrouched = false;
		mMovementSpeed = mWalkSpeed * 2;

		dynamic_cast<CapsuleVolume*>(mBoundingVolume)->SetHalfHeight(1.4f);
	}
}

void PlayerObject::StartSprinting() {
	if (!(mPlayerState == Sprint)) {
		std::cout << "Sprinting" << std::endl;
		mPlayerState = Sprint;
		mIsCrouched = false;
		mMovementSpeed = mSprintSpeed;

		dynamic_cast<CapsuleVolume*>(mBoundingVolume)->SetHalfHeight(1.4f);
	}
}

void PlayerObject::StartCrouching() {
	if (!(mPlayerState == Crouch)) {
		std::cout << "Crouching" << std::endl;
		mPlayerState = Crouch;
		mIsCrouched = true;
		mMovementSpeed = mCrouchSpeed;

		dynamic_cast<CapsuleVolume*>(mBoundingVolume)->SetHalfHeight(0.35f);
		Vector3 temp = GetTransform().GetPosition();
		temp.y -= 2;
		GetTransform().SetPosition(temp);
	}
}

void PlayerObject::MatchCameraRotation() {
	Matrix4 yawRotation = Matrix4::Rotation(mGameWorld->GetMainCamera().GetYaw(), Vector3(0, 1, 0));
	GetTransform().SetOrientation(yawRotation);
}

void PlayerObject::StopSliding() {
	if ((mPhysicsObject->GetLinearVelocity().Length() < 1) && (mPhysicsObject->GetForce() == Vector3(0,0,0))) {
		float fallingSpeed = mPhysicsObject->GetLinearVelocity().y;
		mPhysicsObject->SetLinearVelocity(Vector3(0, fallingSpeed, 0));
	}
}

