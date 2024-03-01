#pragma once
using namespace NCL::Maths;

namespace NCL {
	class CollisionVolume;

	namespace CSC8503 {
		class Transform;

		class PhysicsObject {
		public:
			PhysicsObject(Transform* parentTransform, const CollisionVolume* parentVolume, float inverseMass = 1.0f, float dynamicFriction = 0, float staticFriction = 0, float elasticity = 0.0f);
			~PhysicsObject();

			Vector3 GetLinearVelocity() const {
				return mLinearVelocity;
			}

			Vector3 GetAngularVelocity() const {
				return mAngularVelocity;
			}

			Vector3 GetTorque() const {
				return mTorque;
			}

			Vector3 GetForce() const {
				return mForce;
			}

			void SetInverseMass(float invMass) {
				mInverseMass = invMass;
			}

			float GetInverseMass() const {
				return mInverseMass;
			}

			void ApplyAngularImpulse(const Vector3& force);
			void ApplyLinearImpulse(const Vector3& force);

			void AddForce(const Vector3& force);

			void AddForceAtPosition(const Vector3& force, const Vector3& position);

			void AddTorque(const Vector3& torque);


			void ClearForces();

			void SetLinearVelocity(const Vector3& v) {
				mLinearVelocity = v;
			}

			void SetAngularVelocity(const Vector3& v) {
				mAngularVelocity = v;
			}

			void InitCubeInertia();
			void InitSphereInertia(bool isHollow);

			void UpdateInertiaTensor();

			Matrix3 GetInertiaTensor() const {
				return mInverseInteriaTensor;
			}

			float GetStaticFriction() { return mStaticFriction; }
			float GetDynamicFriction() { return mDynamicFriction; }

			float GetElasticity() { return mElasticity; }

		protected:
			const CollisionVolume* mVolume;
			Transform* mTransform;

			float mInverseMass;
			float mElasticity;
			float mStaticFriction;
			float mDynamicFriction;

			//linear stuff
			Vector3 mLinearVelocity;
			Vector3 mForce;

			//angular stuff
			Vector3 mAngularVelocity;
			Vector3 mTorque;
			Vector3 mInverseInertia;
			Matrix3 mInverseInteriaTensor;
		};
	}
}

