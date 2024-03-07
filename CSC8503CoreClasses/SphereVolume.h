#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class SphereVolume : CollisionVolume
	{
	public:
		SphereVolume(float sphereRadius = 1.0f, const Maths::Vector3& offset = Maths::Vector3(0, 0, 0)) {
			type	= VolumeType::Sphere;
			radius	= sphereRadius;
			this->offset = offset;
			applyPhysics = true;
		}
		SphereVolume(float sphereRadius, bool applyPhysics, const Maths::Vector3& offset = Maths::Vector3(0, 0, 0)) {
			type = VolumeType::Sphere;
			radius = sphereRadius;
			this->offset = offset;
			this->applyPhysics = applyPhysics;
		}
		~SphereVolume() {}

		float GetRadius() const {
			return radius;
		}

		Maths::Vector3 GetOffset() const override {
			return offset;
		}

	protected:
		float	radius;
		Maths::Vector3 offset;
	};
}

