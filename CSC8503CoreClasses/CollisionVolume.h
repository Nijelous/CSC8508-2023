#pragma once
namespace NCL {
	enum class VolumeType {
		AABB	= 1,
		OBB		= 2,
		Sphere	= 4, 
		Mesh	= 8,
		Capsule = 16,
		Compound= 32,
		Invalid = 256
	};

	class CollisionVolume
	{
	public:
		CollisionVolume() {
			type = VolumeType::Invalid;
			applyPhysics = false;
		}
		~CollisionVolume() {}

		virtual void SetHalfHeight(float newHalfHeight) {}

		virtual void SetRadius(float radius) {}

		virtual Maths::Vector3 GetOffset() const { return Maths::Vector3(0, 0, 0); }

		VolumeType type;
		bool applyPhysics;
	};
}