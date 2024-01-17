#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class SphereVolume : CollisionVolume
	{
	public:
		SphereVolume(float sphereRadius = 1.0f) {
			type	= VolumeType::Sphere;
			radius	= sphereRadius;
			applyPhysics = true;
		}
		SphereVolume(float sphereRadius, bool applyPhysics) {
			type = VolumeType::Sphere;
			radius = sphereRadius;
			this->applyPhysics = applyPhysics;
		}
		~SphereVolume() {}

		float GetRadius() const {
			return radius;
		}

	protected:
		float	radius;
	};
}

