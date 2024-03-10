#include "GameObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"
#include "../CSC8503/DebugNetworkedGame.h"
#include "../CSC8503/SceneManager.h"


using namespace NCL::CSC8503;

GameObject::GameObject(CollisionLayer collisionLayer, const std::string& objectName)	{

	mName			= objectName;
	mWorldID			= -1;
	mIsRendered		= true;
	mHasPhysics		= true;
	mBoundingVolume	= nullptr;
	mPhysicsObject	= nullptr;
	mRenderObject	= nullptr;
	mNetworkObject	= nullptr;
	mSoundObject = nullptr;
	mCollisionLayer = collisionLayer;
	
	mObjectState = Idle;

	mIsPlayer = false;
}

GameObject::~GameObject()	{
	delete mBoundingVolume;
	delete mPhysicsObject;
	delete mRenderObject;
#ifdef USEGL
	delete mNetworkObject;
	delete mSoundObject;
#endif

}

#ifdef USEGL
void GameObject::SetIsSensed(bool sensed){
	mRenderObject->SetOutlined(sensed);
}

bool GameObject::GetIsSensed() {
	return mRenderObject->GetOutlined();
}

void GameObject::SetNetworkObject(NetworkObject* netObj) {
	mNetworkObject = netObj;
	netObj->SetGameObject(*this);
}
#endif
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
#ifdef USEPROSPERO
	if (mRenderObject) {
		if(mRenderObject->GetAnimationObject())
			mRenderObject->GetAnimationObject()->Update(dt);
	}
#endif
}

void GameObject::SetObjectState(GameObjectState state) {
	if (mObjectState == state) {
		return;
	}
	
	mObjectState = state;
#ifdef USEGL
	if (mRenderObject->GetAnimationObject() != nullptr ) {
		AnimationSystem* animSystem = LevelManager::GetLevelManager()->GetAnimationSystem();
		animSystem->SetAnimationState(this, mObjectState);
	}

	if (mNetworkObject) {
		SceneManager* sceneManager = SceneManager::GetSceneManager();
		bool isServer = sceneManager->IsServer();
		if (isServer) {
			DebugNetworkedGame* scene = static_cast<DebugNetworkedGame*>(sceneManager->GetCurrentScene());
			scene->SendObjectStatePacket(mNetworkObject->GetnetworkID(), mObjectState);
		}
	}
#endif
}