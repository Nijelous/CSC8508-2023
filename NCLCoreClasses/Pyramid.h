/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include "Plane.h"

namespace NCL::Maths {
	class Matrix4;
	class Vector3;

	class Pyramid {
	public:
		Pyramid(void);
		Pyramid(const Vector3 apex, const Vector3 baseCenter, const float baseL);
		~Pyramid(void) {};		

		bool SphereInsidePyramid(const Vector3& position, float radius) const;

		static Vector3 GetBaseCorner(Vector3 baseCenter, float baseL, int cornerID) ;
	protected:
		Plane mPlanes[5];
	};
}