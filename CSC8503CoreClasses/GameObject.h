#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

using std::vector;

namespace NCL::CSC8503 {
	class NetworkObject;
	class RenderObject;
	class PhysicsObject;
	class AnimationObject;

	class GameObject	{
	public:
		GameObject(const std::string& name = "");
		~GameObject();

		void SetBoundingVolume(CollisionVolume* vol) {
			mBoundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const {
			return mBoundingVolume;
		}

		bool IsActive() const {
			return mIsActive;
		}

		void SetActive() {
			mIsActive = !mIsActive;
		}

		Transform& GetTransform() {
			return mTransform;
		}

		RenderObject* GetRenderObject() const {
			return mRenderObject;
		}

		PhysicsObject* GetPhysicsObject() const {
			return mPhysicsObject;
		}

		NetworkObject* GetNetworkObject() const {
			return mNetworkObject;
		}
    
    void SetNetworkObject(NetworkObject* netObj) { mNetworkObject = netObj; }
    
		AnimationObject* GetAnimationObject() const {
			return mAnimationObject;
		}

		void SetRenderObject(RenderObject* newObject) {
			mRenderObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			mPhysicsObject = newObject;
		}

		void SetAnimationObject(AnimationObject* newObject) {
			mAnimationObject = newObject;
		}

		const std::string& GetName() const {
			return mName;
		}

		virtual void OnCollisionBegin(GameObject* otherObject) {
			//std::cout << "OnCollisionBegin event occured!\n";
		}

		virtual void OnCollisionEnd(GameObject* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		bool GetBroadphaseAABB(Vector3&outsize) const;

		void UpdateBroadphaseAABB();

		void SetWorldID(int newID) {
			mWorldID = newID;
		}

		int		GetWorldID() const {
			return mWorldID;
		}
    
		virtual void UpdateObject(float dt);

		bool GetIsPlayer() { return mIsPlayer; }

	protected:
		Transform			mTransform;

		CollisionVolume*	mBoundingVolume;
		PhysicsObject*		mPhysicsObject;
		RenderObject*		mRenderObject;
		NetworkObject*		mNetworkObject;
		AnimationObject* mAnimationObject;

		bool		mIsActive;
		int			mWorldID;
		std::string	mName;

		Vector3 mBroadphaseAABB;

		bool mIsPlayer;
	};
}

