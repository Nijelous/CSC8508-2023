#include "GuardObject.h"
#include "PlayerObject.h"
#include "Ray.h"
#include "Debug.h"

using namespace NCL;
using namespace CSC8503;

GuardObject::GuardObject(const std::string& objectName) {
	mName = objectName;
}

GuardObject::~GuardObject() {
	delete mPlayer;
	delete mWorld;
}

void GuardObject::UpdateObject(float dt) {
	RaycastToPlayer();
}

void GuardObject::RaycastToPlayer() {
	mPlayer = dynamic_cast<PlayerObject*>(mPlayer);
	Vector3 dir = (mPlayer->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Normalised();
	RayCollision closestCollision;
	Ray r = Ray(this->GetTransform().GetPosition(), dir);
	float ang = Vector3::Dot(dir, AngleOfSight());
	if (ang > 2) {
		if (mWorld->Raycast(r, closestCollision, true, this)) {
			mSightedObject = (GameObject*)closestCollision.node;
			Debug::DrawLine(this->GetTransform().GetPosition(), closestCollision.collidedAt);
			if (mSightedObject == mPlayer) {
				std::cout << "Gotem";
			}
		}
	}
}

Vector3 GuardObject::AngleOfSight() {
	Vector3 rightAxis = this->transform.GetMatrix().GetColumn(0);
	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	return fwdAxis;
}