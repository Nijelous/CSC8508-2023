/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

namespace NCL::Maths {
	class Matrix3;
	class Matrix4;
	class Vector3;

	class Quaternion {
	public:
		float x;
		float y;
		float z;
		float w;

	public:
		Quaternion(void);
		Quaternion(float x, float y, float z, float w);
		Quaternion(double x, double y, double z, double w);
		Quaternion(const Vector3& vector, float w);

		Quaternion(const Matrix3 &m);
		Quaternion(const Matrix4 &m);

		~Quaternion(void) = default;

		void		Normalise(); 
		Quaternion	Normalised() const;

			
		static float Dot(const Quaternion &a, const Quaternion &b);

		static Quaternion	Lerp(const Quaternion &from, const Quaternion &to, float by);
		static Quaternion	Slerp(const Quaternion &from, const Quaternion &to, float by);

		Vector3		ToEuler() const;
		Quaternion	Conjugate() const;
		void		CalculateW();	//builds 4th component when loading in shortened, 3 component quaternions
		Vector3		ToVector3() const;

		static Quaternion EulerAnglesToQuaternion(float pitch, float yaw, float roll);
		static Quaternion AxisAngleToQuaterion(const Vector3& vector, float degrees);

		inline bool  operator ==(const Quaternion &from)	const {
			if (x != from.x || y != from.y || z != from.z || w != from.w) {
				return false;
			}
			return true;
		}

		inline bool  operator !=(const Quaternion &from)	const {
			if (x != from.x || y != from.y || z != from.z || w != from.w) {
				return true;
			}
			return false;
		}

		inline Quaternion  operator *(const Quaternion &b)	const {
			return Quaternion(
				(x * b.w) + (w * b.x) + (y * b.z) - (z * b.y),
				(y * b.w) + (w * b.y) + (z * b.x) - (x * b.z),
				(z * b.w) + (w * b.z) + (x * b.y) - (y * b.x),
				(w * b.w) - (x * b.x) - (y * b.y) - (z * b.z)
			);
		}

		Vector3		operator *(const Vector3 &a)	const;

		inline Quaternion  operator *(const float &a)		const {
			return Quaternion(x*a, y*a, z*a, w*a);
		}

		inline Quaternion  operator *=(const float &a) {
			*this = *this * a;
			return *this;
		}

		inline Quaternion  operator -()	const {
			return Quaternion(-x, -y, -z, -w);
		}

		inline Quaternion  operator -(const Quaternion &a)	const {
			return Quaternion(x - a.x, y - a.y, z - a.z, w - a.w);
		}

		inline Quaternion  operator -=(const Quaternion &a) {
			*this = *this - a;
			return *this;
		}

		inline Quaternion  operator +(const Quaternion &a)	const {
			return Quaternion(x + a.x, y + a.y, z + a.z, w + a.w);
		}

		inline Quaternion  operator +=(const Quaternion &a) {
			*this = *this + a;
			return *this;
		}

		inline float operator[](int i) const {
			return ((float*)this)[i];
		}

		inline float& operator[](int i) {
			return ((float*)this)[i];
		}

		inline friend std::ostream& operator <<(std::ostream& o, const Quaternion& q);
		inline friend std::istream& operator >>(std::istream& i, Quaternion &v);

		static Matrix4 RotationMatrix(const Quaternion& quat);
	};

	std::ostream& operator<<(std::ostream& o, const Quaternion& q) {
		o	<< "Quaternion("
			<< q.x; o << ","
			<< q.y; o << ","
			<< q.z; o << ","
			<< q.w;
		return o;
	}

	std::istream& operator >> (std::istream& i, Quaternion &v) {
		char ignore;
		i >> std::skipws >> v.x >> ignore >> v.y >> ignore >> v.z >> ignore >> v.w >> std::noskipws;
		return i;
	}
}

template <>
struct std::hash<NCL::Maths::Quaternion>
{
	std::size_t operator()(const NCL::Maths::Quaternion& key) const {
		size_t seed = 0;
		hashCombine(seed, std::hash<float>()(key.x));
		hashCombine(seed, std::hash<float>()(key.y));
		hashCombine(seed, std::hash<float>()(key.z));
		hashCombine(seed, std::hash<float>()(key.w));
		return seed;
	}
private:
	void hashCombine(size_t& seed, size_t hash) const {
		seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
};