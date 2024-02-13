#include "Helipad.h"
#include "PlayerObject.h"
#include "CollisionDetection.h"

using namespace NCL::CSC8503;

Helipad::Helipad() : GameObject(StaticObj, "Helipad") {

}

void Helipad::OnCollisionBegin(GameObject* otherObject) {
	if (PlayerObject* temp = dynamic_cast<PlayerObject*>(otherObject)) {
		mCollidingWithPlayer = true;
	}
}

void Helipad::OnCollisionEnd(GameObject* otherObject) {
	if (PlayerObject* temp = dynamic_cast<PlayerObject*>(otherObject)) {
		mCollidingWithPlayer = false;
	}
}
