#pragma once
#include "GameWorld.h"

namespace NCL {
	namespace CSC8503 {
		class PhysicsSystem	{
		public:
			PhysicsSystem(GameWorld& g);
			~PhysicsSystem();

			void Clear();

			void Update(float dt);

			void UseGravity(bool state) {
				mApplyGravity = state;
			}

			void SetGlobalDamping(float d) {
				mGlobalDamping = d;
			}

			void SetGravity(const Vector3& g);
		protected:
			void BasicCollisionDetection();
			void BroadPhase();
			void NarrowPhase();

			void ClearForces();

			void IntegrateAccel(float dt);
			void IntegrateVelocity(float dt);

			void UpdateConstraints(float dt);

			void UpdateCollisionList();
			void UpdateObjectAABBs();

			float ImpulseResolveCollision(GameObject& a , GameObject&b, CollisionDetection::ContactPoint& p) const;

			void FrictionImpulse(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p, float j) const;

			bool GetIsCapsule(GameObject& obj) const;

			void SeperateObjects(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p, float totalMass) const;

			bool CheckFrictionShuldBeApplied(float aVelocity, float bVelocity, float totalMass) const;

			Vector3 CalculateCollisionVelocity(Vector3 contactPointA, Vector3 contactPointB, PhysicsObject* physA, PhysicsObject* physB) const;

			Vector3 CalculateFrictionDirection(Vector3 contactVelocity, Vector3 collisionNormal) const;

			float CalculateInertia(PhysicsObject* physA, PhysicsObject* physB, Vector3 contactPointA, Vector3 contactPointB, Vector3 angle) const;

			float CalculateFriction(PhysicsObject* physA, PhysicsObject* physB) const;

			Vector3 CalculateFrictionImpulse(Vector3 tangent, float j, float friction) const;

			GameWorld& mGameWorld;

			bool	mApplyGravity;
			Vector3 mGravity;
			float	mDTOffset;
			float	mGlobalDamping;

			std::set<CollisionDetection::CollisionInfo> mAllCollisions;
			std::set<CollisionDetection::CollisionInfo> mBroadphaseCollisions;
			std::vector<CollisionDetection::CollisionInfo> mBroadphaseCollisionsVec;
			bool mUseBroadPhase		= true;
			int mNumCollisionFrames	= 5;
		};
	}
}

