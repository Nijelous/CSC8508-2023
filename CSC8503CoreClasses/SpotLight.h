#pragma once
#include "PointLight.h"

using namespace NCL;
using namespace Maths;

class SpotLight : public PointLight
{
public:
	/*
	Spotlights illuminate an angle with light. They consist of a bright inner ring, and a small & dimmer outer ring, to give a softer edge. 
	The 'degreesDimming' parameter sets the difference between these two rings.  
	*/
	SpotLight(const Vector3& dir, const Vector3& pos, const Vector4& colour, float radius, float angle, float degreesDimming)
		:PointLight(pos, colour, radius)	{
		mAngle = angle;
		mDirection = dir.Normalised();
		mDotProdMin = cosf((angle / 2) * (PI / 180));
		mDimProdMin = cosf(((angle - degreesDimming) / 2) * (PI / 180));
		mType = Spot;
	}
	~SpotLight() {};
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

	const float* GetDotProdMinAddress() const {
		const float* address = &mDotProdMin;
		return address;
	}

	const float* GetDimProdMinAddress() const {
		const float* address = &mDimProdMin;
		return address;
	}

	const float* GetDirectionAddress() const {
		const float* address = &mDirection.x;
		return address;
	}

	const float* GetRadiusAddress() const {
		const float* address = &mRadius;
		return address;
	}

protected:
	float mAngle;
	float mDotProdMin;
	float mDimProdMin;	
	Vector3 mDirection;
};