#include "GameObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"
#include "PlayerObject.h"

#include "Window.h"
#include "GameWorld.h"

using namespace NCL::CSC8503;

PlayerObject::PlayerObject(GameWorld* world, const std::string& objName, int movementSpeed) {
	mName = objName;
	mMovementSpeed = movementSpeed;
	mGameWorld = world;

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
	
	StopSliding();
}

void PlayerObject::MatchCameraRotation() {
	Matrix4 yawRotation = Matrix4::Rotation(mGameWorld->GetMainCamera().GetYaw(), Vector3(0, 1, 0));
	GetTransform().SetOrientation(yawRotation);
}

void PlayerObject::StopSliding() {
	if ((mPhysicsObject->GetLinearVelocity().Length() < 1) && (mPhysicsObject->GetForce() == Vector3(0,0,0))) {
		mPhysicsObject->SetLinearVelocity(Vector3(0, 0, 0));
	}
}

