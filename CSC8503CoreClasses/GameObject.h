#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

using std::vector;

namespace NCL::CSC8503 {
	class NetworkObject;
	class PhysicsObject;
	class SoundObject;
	class RenderObject;

	enum CollisionLayer {
		StaticObj = 1,
		Collectable = 2,
		Player = 4,
		Npc = 8,
		NoCollide = 16,
		Zone = 32,
		NoSpecialFeatures = 64
	};

	class GameObject {
	public:
		GameObject(CollisionLayer = NoSpecialFeatures, const std::string& name = "");
		~GameObject();

		enum GameObjectState {
			Idle,
			Walk,
			Sprint,
			IdleCrouch,
			Crouch,
			Happy,
			Point,
			Default
		};

		void SetBoundingVolume(CollisionVolume* vol) {
			mBoundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const {
			return mBoundingVolume;
		}

		bool IsRendered() const {
			return mIsRendered;
		}

		bool HasPhysics() const {
			return mHasPhysics;
		}

		void SetIsRendered(bool isRendered) {
			mIsRendered = isRendered;
		}

		void SetHasPhysics(bool hasPhysics) {
			mHasPhysics = hasPhysics;
		}

		void ToggleIsRendered() {
			mIsRendered = !mIsRendered;
		}

		void ToggleHasPhysics() {
			mHasPhysics = !mHasPhysics;
		}

		void SetActive(bool isActive) {
			mIsRendered = isActive;
			mHasPhysics = isActive;
		}

		bool IsActive() {
			return mIsRendered && mHasPhysics;
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

		SoundObject* GetSoundObject() const {
			return mSoundObject;
		}


#ifdef USEGL
		void SetIsSensed(bool sensed);

		bool GetIsSensed();
#endif


		void SetNetworkObject(NetworkObject* netObj);


		void SetRenderObject(RenderObject* newObject) {
			mRenderObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			mPhysicsObject = newObject;
		}



		void SetSoundObject(SoundObject* newObject) {
			mSoundObject = newObject;
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

		bool GetBroadphaseAABB(Vector3& outsize) const;

		void UpdateBroadphaseAABB();

		void SetWorldID(int newID) {
			mWorldID = newID;
		}

		int		GetWorldID() const {
			return mWorldID;
		}

		virtual void UpdateObject(float dt);

		bool GetIsPlayer() { return mIsPlayer; }

		CollisionLayer GetCollisionLayer() {
			return mCollisionLayer;
		}

		void SetCollisionLayer(CollisionLayer collisionLayer) {
			mCollisionLayer = collisionLayer;
		}

		GameObjectState GetGameOjbectState() {
			return mObjectState;
		}

		void SetName(const std::string& name) {
			mName = name;
		}

		void SetObjectState(GameObjectState state);

		void DrawCollisionVolume();

		const std::string& GetGameObjectStateStr() const;

	protected:
		Transform			mTransform;

		CollisionVolume* mBoundingVolume;
		PhysicsObject* mPhysicsObject;
		RenderObject* mRenderObject;
		NetworkObject* mNetworkObject;
		SoundObject* mSoundObject;

		bool		mIsSensed;
		bool		mHasPhysics;
		bool		mIsRendered;
		int			mWorldID;
		std::string	mName;

		Vector3 mBroadphaseAABB;

		CollisionLayer mCollisionLayer;
		bool mIsPlayer;

		GameObjectState mObjectState;
	};
}

