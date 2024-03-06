#include "Helipad.h"
#include "PlayerObject.h"
#include "CollisionDetection.h"

using namespace NCL::CSC8503;

Helipad::Helipad() : GameObject(StaticObj, "Helipad") {
	mCollidingPlayerID = -1;
}

void Helipad::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->GetCollisionLayer() == CollisionLayer::Player) {
		PlayerObject* temp = static_cast<PlayerObject*>(otherObject);
		mCollidingWithPlayer = true;
		mCollidingPlayerID = temp->GetPlayerID();
	}
}

void Helipad::OnCollisionEnd(GameObject* otherObject) {
	if (otherObject->GetCollisionLayer() == CollisionLayer::Player) {
		PlayerObject* temp = static_cast<PlayerObject*>(otherObject);
		mCollidingWithPlayer = false;
		mCollidingPlayerID = -1;
	}
}
