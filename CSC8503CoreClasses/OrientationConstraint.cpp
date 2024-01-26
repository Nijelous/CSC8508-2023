#include "OrientationConstraint.h"
#include "GameObject.h"
#include "PhysicsObject.h"
using namespace NCL;
using namespace Maths;
using namespace CSC8503;

OrientationConstraint::OrientationConstraint(GameObject* a, GameObject* b, float maxAngleDiff)
{
	objectA = a;
	objectB = b;
	angle = maxAngleDiff;
}

OrientationConstraint::~OrientationConstraint()
{

}

// Ensures that two objects orientations are constrained if an orientation constraint is created
//
// Author: Ewan Squire
void OrientationConstraint::UpdateConstraint(float dt) {
	Vector3 eulerA = objectA->GetTransform().GetOrientation().ToEuler();
	Vector3 eulerB = objectB->GetTransform().GetOrientation().ToEuler();

	Vector3 relativeOri = eulerA - eulerB;

	float angleDifference = relativeOri.Length();

	float offset = angle - angleDifference;

	if (abs(offset) > 0.0f) {
		Vector3 offsetDir = relativeOri.Normalised();

		PhysicsObject* physA = objectA->GetPhysicsObject();
		PhysicsObject* physB = objectB->GetPhysicsObject();

		Vector3 relativeVelocity = physA->GetAngularVelocity() - physB->GetAngularVelocity();
		float constraintMass = physA->GetInverseMass() + physB->GetInverseMass();

		// if at least one object is not static
		if (constraintMass > 0.0f) {
			float velocityDot = Vector3::Dot(relativeVelocity, offsetDir);
			float biasFactor = 0.01f;
			float bias = -(biasFactor / dt) * offset;
			float lambda = -(velocityDot + bias) / constraintMass;

			Vector3 aImpulse = offsetDir * lambda;
			Vector3 bImpulse = -offsetDir * lambda;

			physA->ApplyAngularImpulse(aImpulse);
			physB->ApplyAngularImpulse(bImpulse);
		}
	}
}