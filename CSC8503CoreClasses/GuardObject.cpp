#include "GuardObject.h"
#include "PlayerObject.h"
#include "Ray.h"
#include "Debug.h"

using namespace NCL;
using namespace CSC8503;

GuardObject::GuardObject(const std::string& objectName) {
	name = objectName;
}

GuardObject::~GuardObject() {

}

void GuardObject::UpdateObject(float dt) {
	RaycastToPlayer();
}

void GuardObject::RaycastToPlayer() {
	mPlayer = dynamic_cast<PlayerObject*>(mPlayer);
	Vector3 dir = (mPlayer->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Normalised();
	RayCollision closestCollision;
	Ray r = Ray(this->GetTransform().GetPosition(), dir);
	if (mWorld->Raycast(r, closestCollision, true, this)) {
		Debug::DrawLine(this->GetTransform().GetPosition(), closestCollision.collidedAt);
	}
}