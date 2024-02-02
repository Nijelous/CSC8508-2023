#pragma once
#include "PointLight.h"

using namespace NCL;
using namespace Maths;

class Spotlight : public PointLight
{
public:
	/*
	Spotlights illuminate an angle with light. They consist of a bright inner ring, and a small & dimmer outer ring, to give a softer edge. 
	The 'degreesDimming' parameter sets the difference between these two rings.  
	*/
	Spotlight(const Vector3& dir, const Vector3& pos, const Vector4& colour, float radius, float angle, float degreesDimming)
		:PointLight(pos, colour, radius)	{
		mAngle = angle;
		mDirection = dir;
		mDotProdMin = cosf((angle / 2) * (PI / 180));
		mDimProdMin = cosf(((angle - degreesDimming) / 2) * (PI / 180));
		mDirection = Vector3(0, 0, -1);
		mName = "spot";
	}
	~Spotlight() {};
	float GetAngle() const { 
		return mAngle; 
	}
	void SetAngle(float angle) { 
		mAngle = angle; 
		mDotProdMin = cosf(angle / 2); 
	}
	Vector3 GetDirection() const { 
		return mDirection; 
	}
	void SetDirection(const Vector3& dir) { 
		mDirection = dir; 
	}
	float GetDotProdMin() const { 
		return mDotProdMin; 
	}
	float GetDimProdMin() const { 
		return mDimProdMin; 
	}

protected:
	float mAngle;
	float mDotProdMin;
	float mDimProdMin;	
	Vector3 mDirection;
};