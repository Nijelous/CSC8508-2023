#pragma once
#include "BaseLight.h"

using namespace NCL;
using namespace Maths;

class DirectionLight : public Light
{
public:
	DirectionLight(Vector3 direction, const Vector4& colour) {
		direction.Normalise();
		this->mDirection = direction;
		this->mColour = colour;
		mType = Direction;
	}

	~DirectionLight(void) {};
	Vector3 GetDirection() const { 
		return mDirection; 
	}
	void SetDirection(const Vector3& val) { 
		mDirection = val; 
	}

protected:
	Vector3 mDirection;
};