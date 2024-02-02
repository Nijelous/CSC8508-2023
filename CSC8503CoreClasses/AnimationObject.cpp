#include "AnimationObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"



AnimationObject::AnimationObject(const std::string& objectName) {
	mName = objectName;
	mWorldID = 2;
	mIsActive = true;
	mBoundingVolume = nullptr;
	mPhysicsObject = nullptr;
	mRenderObject = nullptr;
	mNetworkObject = nullptr;
	mAnimation = nullptr;
	mMaterial = nullptr;
	
	
}

AnimationObject::~AnimationObject() {
	delete mBoundingVolume;
	delete mPhysicsObject;
	delete mRenderObject;
	delete mNetworkObject;
	delete mAnimation;
	delete mMaterial;
}

void AnimationObject::setMeshAnimationRun() {
	NCL::Rendering::MeshAnimation* animation = new MeshAnimation("Role_T.anm");
	//I am not sure is it the best way to handle the error about different namespace!
	
}
