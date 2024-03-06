#pragma once

using std::vector;

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class Transform
		{
		public:
			Transform();
			~Transform();

			Transform& SetPosition(const Vector3& worldPos);
			Transform& SetScale(const Vector3& worldScale);
			Transform& SetOrientation(const Quaternion& newOr);

			Vector3 GetPosition() const {
				return position;
			}

			Vector3 GetScale() const {
				return scale;
			}

			Quaternion GetOrientation() const {
				return orientation;
			}

			Matrix4 GetMatrix() const {
				return mMatrix;
			}

			void SetMatrix(Matrix4 matrix) {
				mMatrix = matrix;
				UpdateVariables();
			}
			void UpdateMatrix();

			void UpdateVariables();

			inline bool operator<(const Transform& rhs) const { return this->GetPosition().x == rhs.GetPosition().x ? 
				(this->GetPosition().y == rhs.GetPosition().y ? this->GetPosition().z < rhs.GetPosition().z : this->GetPosition().y < rhs.GetPosition().y) 
				: this->GetPosition().x < rhs.GetPosition().x; }

			inline bool	operator==(const Transform& rhs)const { return (this->GetPosition() == rhs.GetPosition() && 
				this->GetOrientation() == rhs.GetOrientation() &&
				this->GetScale() == rhs.GetScale()) ? true : false; };
		protected:
			Matrix4		mMatrix;
			Quaternion	orientation;
			Vector3		position;

			Vector3		scale;
		};
	}
}

template <>
struct std::hash<NCL::CSC8503::Transform>
{
	std::size_t operator()(const NCL::CSC8503::Transform& key) const {
		size_t seed = 0;
		hashCombine(seed, std::hash<NCL::Maths::Vector3>()(key.GetPosition()));
		hashCombine(seed, std::hash<NCL::Maths::Quaternion>()(key.GetOrientation()));
		hashCombine(seed, std::hash<NCL::Maths::Vector3>()(key.GetScale()));
		return seed;
	}
private:
	void hashCombine(size_t& seed, size_t hash) const {
		seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
};
