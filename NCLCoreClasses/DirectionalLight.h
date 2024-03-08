#pragma once
#include "BaseLight.h"
#include <Transform.h>

using namespace NCL;
using namespace Maths;

class DirectionLight : public Light
{
public:
	DirectionLight(const Vector3& direction, const Vector4& colour, float radius, const Vector3& centre) {
		this->mDirection = direction.Normalised();
		this->mColour = colour;
		mRadius = radius;
		mCentre = centre;
		mType = Direction;
	}

	~DirectionLight(void) {};
	Vector3 GetDirection() const { 
		return mDirection; 
	}
	void SetDirection(const Vector3& val) { 
		mDirection = val; 
	}

	float GetRadius() {
		return mRadius;
	}

	Vector3 GetCentre() const {
		return mCentre;
	}

	const float* GetDirectionAddress() const {
		const float* address = &mDirection.x;
		return address;
	}

	const float* GetCentreAddress() const {
		const float* address = &mCentre.x;
		return address;
	}

	const float* GetRadiusAddress() const {
		const float* address = &mRadius;
		return address;
	}

protected:
	Vector3 mDirection;
	float mRadius;
	Vector3 mCentre;
};