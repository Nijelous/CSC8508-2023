#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class OBBVolume : CollisionVolume
	{
	public:
		OBBVolume(const Maths::Vector3& halfDims, const Maths::Vector3& offset = Maths::Vector3(0, 0, 0)) {
			type		= VolumeType::OBB;
			halfSizes	= halfDims;
			this->offset = halfDims;
			this->applyPhysics = true;
		}
		~OBBVolume() {}

		Maths::Vector3 GetHalfDimensions() const {
			return halfSizes;
		}
		Maths::Vector3 GetOffset() const override {
			return offset;
		}
	protected:
		Maths::Vector3 halfSizes;
		Maths::Vector3 offset;
	};
}

