/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Pyramid.h"
#include "Matrix4.h"
#include "Vector4.h"

using namespace NCL;
using namespace NCL::Maths;

Pyramid::Pyramid(void) {

}

//Create a pyramid with a given apex and a flat base
Pyramid::Pyramid(const Vector3 apex, const Vector3 baseCenter, const float baseL){
	//Base Corners
	Vector3 baseCorners[4];
	for (int i = 0; i < 4; i++)
		baseCorners[i] = GetBaseCorner(baseCenter, baseL,i);
	//Faces
	mPlanes[1] = Plane::PlaneFromTri(baseCorners[0], apex, baseCorners[1]);
	mPlanes[2] = Plane::PlaneFromTri(baseCorners[1], apex, baseCorners[2]);
	mPlanes[3] = Plane::PlaneFromTri(baseCorners[2], apex, baseCorners[3]);
	mPlanes[4] = Plane::PlaneFromTri(baseCorners[3], apex, baseCorners[0]);
}

bool NCL::Maths::Pyramid::SphereInsidePyramid(const Vector3& position, float radius) const{
	for (int p = 0; p < 5; ++p) {
		if (!mPlanes[p].SphereInPlane(position, radius)) {
			return false;
		}
	}
	return true;
}

Vector3 NCL::Maths::Pyramid::GetBaseCorner(Vector3 baseCenter, float baseL, int cornerID)
{
	const float halfL = baseL / 2;

	switch (cornerID){
	case 0:
		return baseCenter + Vector3(-halfL, 0, -halfL);
	case 1:
		return baseCenter + Vector3(-halfL, 0, halfL);
	case 2:
		return baseCenter + Vector3(halfL, 0, halfL);
	case 3:
		return baseCenter + Vector3(halfL, 0, -halfL);
	}

	return Vector3();
}
;
