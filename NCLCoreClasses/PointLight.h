#pragma once
#include "BaseLight.h"

using namespace NCL;
using namespace Maths;

class PointLight : public Light
{
public:
	PointLight(const Vector3& position, const Vector4& colour, float radius) {
		this->mPosition = position;
		this->mColour = colour;
		this->mRadius = radius;
		mType = Point;
	}

	~PointLight() {};
	Vector3 GetPosition() const { 
		return mPosition; 
	}
	
	const float *GetPositionAddress() const {
		const float* address = &mPosition.x;
		return address;
	}

	void SetPosition(const Vector3& val) { 
		mPosition = val; 
	}
	float GetRadius() const { 
		return mRadius; 
	}
	void SetRadius(float val) { 
		mRadius = val; 
	}

	const float* GetRadiusAddress() const {
		const float* address = &mRadius;
		return address;
	}

protected:
	Vector3 mPosition;
	float mRadius;
};