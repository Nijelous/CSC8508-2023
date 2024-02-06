#include "PhysicsObject.h"
#include "PhysicsSystem.h"
#include "Transform.h"
using namespace NCL;
using namespace CSC8503;

PhysicsObject::PhysicsObject(Transform* parentTransform, const CollisionVolume* parentVolume, float inverseMass, float dynamicFriction, float staticFriction, float elasticity)	{
	mTransform	= parentTransform;
	mVolume		= parentVolume;

	mInverseMass = inverseMass;
	mElasticity = elasticity;
	// friction when stopping
	mStaticFriction = staticFriction;
	// friction when moving
	mDynamicFriction = dynamicFriction;
}

PhysicsObject::~PhysicsObject()	{

}

void PhysicsObject::ApplyAngularImpulse(const Vector3& force) {
	mAngularVelocity += mInverseInteriaTensor * force;
}

void PhysicsObject::ApplyLinearImpulse(const Vector3& force) {
	mLinearVelocity += force * mInverseMass;
}

void PhysicsObject::AddForce(const Vector3& addedForce) {
	mForce += addedForce;
}

void PhysicsObject::AddForceAtPosition(const Vector3& addedForce, const Vector3& position) {
	Vector3 localPos = position - mTransform->GetPosition();

	mForce  += addedForce;
	mTorque += Vector3::Cross(localPos, addedForce);
}

void PhysicsObject::AddTorque(const Vector3& addedTorque) {
	mTorque += addedTorque;
}

void PhysicsObject::ClearForces() {
	mForce				= Vector3();
	mTorque				= Vector3();
}

void PhysicsObject::InitCubeInertia() {
	Vector3 dimensions	= mTransform->GetScale();

	Vector3 fullWidth = dimensions * 2;

	Vector3 dimsSqr		= fullWidth * fullWidth;

	mInverseInertia.x = (12.0f * mInverseMass) / (dimsSqr.y + dimsSqr.z);
	mInverseInertia.y = (12.0f * mInverseMass) / (dimsSqr.x + dimsSqr.z);
	mInverseInertia.z = (12.0f * mInverseMass) / (dimsSqr.x + dimsSqr.y);
}

void PhysicsObject::InitSphereInertia(bool isHollow) {
	float radius	= mTransform->GetScale().GetMaxElement();
	float i;
	if (!isHollow)
		i = 2.5f * mInverseMass / (radius*radius);
	else
		i = 1.3f * mInverseMass / (radius * radius);

	mInverseInertia	= Vector3(i, i, i);
}

void PhysicsObject::UpdateInertiaTensor() {
	Quaternion q = mTransform->GetOrientation();
	
	Matrix3 invOrientation	= Matrix3(q.Conjugate());
	Matrix3 orientation		= Matrix3(q);

	mInverseInteriaTensor = orientation * Matrix3::Scale(mInverseInertia) *invOrientation;
}