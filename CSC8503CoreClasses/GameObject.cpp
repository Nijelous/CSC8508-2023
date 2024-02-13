#include "GameObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"
#include "AnimationObject.h"

using namespace NCL::CSC8503;

GameObject::GameObject(CollisionLayer collisionLayer, const std::string& objectName)	{

	mName			= objectName;
	mWorldID			= -1;
	mIsActive		= true;
	mBoundingVolume	= nullptr;
	mPhysicsObject	= nullptr;
	mRenderObject	= nullptr;
	mNetworkObject	= nullptr;
  mAnimationObject = nullptr;
  mCollisionLayer = collisionLayer;

	mIsPlayer = false;
}

GameObject::~GameObject()	{
	delete mBoundingVolume;
	delete mPhysicsObject;
	delete mRenderObject;
	delete mNetworkObject;
	 delete mAnimationObject;
}

bool GameObject::GetBroadphaseAABB(Vector3&outSize) const {
	if (!mBoundingVolume) {
		return false;
	}
	outSize = mBroadphaseAABB;
	return true;
}

void GameObject::UpdateBroadphaseAABB() {
	if (!mBoundingVolume) {
		return;
	}
	if (mBoundingVolume->type == VolumeType::AABB) {
		mBroadphaseAABB = ((AABBVolume&)*mBoundingVolume).GetHalfDimensions();
	}
	else if (mBoundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*mBoundingVolume).GetRadius();
		mBroadphaseAABB = Vector3(r, r, r);
	}
	else if (mBoundingVolume->type == VolumeType::OBB) {
		Matrix3 mat = Matrix3(mTransform.GetOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = ((OBBVolume&)*mBoundingVolume).GetHalfDimensions();
		mBroadphaseAABB = mat * halfSizes;
	}
	else if (mBoundingVolume->type == VolumeType::Capsule) {
		mBroadphaseAABB = Vector3(((CapsuleVolume&)*mBoundingVolume).GetRadius(),
			((CapsuleVolume&)*mBoundingVolume).GetHalfHeight(),
			((CapsuleVolume&)*mBoundingVolume).GetRadius());
	}
}

void GameObject::UpdateObject(float dt) {

}