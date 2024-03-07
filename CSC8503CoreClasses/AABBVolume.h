#pragma once
#include "CollisionVolume.h"
#include "Vector3.h"

namespace NCL {
	using namespace NCL::Maths;
	class AABBVolume : CollisionVolume
	{
	public:
		AABBVolume(const Vector3& halfDims, const Maths::Vector3& offset = Maths::Vector3(0, 0, 0)) {
			type		= VolumeType::AABB;
			halfSizes	= halfDims;
			this->offset = offset;
			this->applyPhysics = true;
		}
		~AABBVolume() {

		}

		Vector3 GetHalfDimensions() const {
			return halfSizes;
		}

		Vector3 GetOffset() const override {
			return offset;
		}

	protected:
		Vector3 halfSizes;
		Vector3 offset;
	};
}
