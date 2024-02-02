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
	name = objName;
	mMovementSpeed = movementSpeed;
	mGameWorld = world;
}

PlayerObject::~PlayerObject() {

}

void PlayerObject::UpdateObject(float dt)
{
	MovePlayer(dt);
	AttachCameraToPlayer(mGameWorld);
}

void PlayerObject::AttachCameraToPlayer(GameWorld* world) {
	Vector3 offset = GetTransform().GetPosition();
	offset.y += 3;
	world->GetMainCamera().SetPosition(offset);
}

void PlayerObject::MovePlayer(float dt) {
	Vector3 fwdAxis = GetForwardAxis();
	Vector3 rightAxis = GetRightAxis();

	if (Window::GetKeyboard()->KeyDown(KeyCodes::W))
		physicsObject->AddForce(-fwdAxis * mMovementSpeed);

	if (Window::GetKeyboard()->KeyDown(KeyCodes::S))
		physicsObject->AddForce(fwdAxis * mMovementSpeed);

	if (Window::GetKeyboard()->KeyDown(KeyCodes::A))
		physicsObject->AddForce(rightAxis * mMovementSpeed);

	if (Window::GetKeyboard()->KeyDown(KeyCodes::D))
		physicsObject->AddForce(-rightAxis * mMovementSpeed);
}

Vector3 PlayerObject::GetForwardAxis() {
	// to be changed
	return Vector3(0,0,-1);
}

Vector3 PlayerObject::GetRightAxis() {
	// to be changed
	return Vector3(1,0,0);
}
