#include "GameObject.h"

#include "AnimationSystem.h"
#include "../CSC8503/LevelManager.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"
#include "../CSC8503/LevelManager.h"
#include "../CSC8503/DebugNetworkedGame.h"
#include "../CSC8503/SceneManager.h"
#include "Debug.h"


using namespace NCL::CSC8503;

namespace {
	std::map<GameObject::GameObjectState, std::string> objectStateStrMap = {
		{GameObject::GameObjectState::Idle, "Idle"},
		{GameObject::GameObjectState::Walk, "Walk"},
		{GameObject::GameObjectState::Crouch, "Crouch"},
		{GameObject::GameObjectState::Sprint, "Sprint"},
		{GameObject::GameObjectState::Happy, "Happy"},
		{GameObject::GameObjectState::IdleCrouch, "IdleCrouch"},
		{GameObject::GameObjectState::Point, "Point"},
		{GameObject::GameObjectState::Default, "Default"},
	};
	
}

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
	if (mRenderObject->GetAnimationObject() != nullptr ) {
		AnimationSystem* animSystem = LevelManager::GetLevelManager()->GetAnimationSystem();
		animSystem->SetAnimationState(this, mObjectState);
	}

#ifdef USEGL
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

void GameObject::DrawCollisionVolume() {
	if (!mBoundingVolume) return;
	switch (mBoundingVolume->type) {
	case VolumeType::AABB:
	{
		AABBVolume* volume = (AABBVolume*)mBoundingVolume;
		Debug::DrawCube(volume->GetHalfDimensions(), volume->GetOffset() + mTransform.GetPosition());
	}
		break;
	case VolumeType::OBB:
	{
		OBBVolume* volume = (OBBVolume*)mBoundingVolume;
		Debug::DrawRotatedCube(volume->GetHalfDimensions(), volume->GetOffset() + mTransform.GetPosition(), mTransform.GetOrientation());
	}
		break;
	case VolumeType::Sphere:
	{
		SphereVolume* volume = (SphereVolume*)mBoundingVolume;
		Debug::DrawSphere(volume->GetRadius(), volume->GetOffset() + mTransform.GetPosition());
	}
		break;
	}
}

const std::string& GameObject::GetGameObjectStateStr() const {
	return objectStateStrMap[mObjectState];
}
