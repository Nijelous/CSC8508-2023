#pragma once
#include "BaseLight.h"

using namespace NCL;
using namespace Maths;

class PointLight : public BaseLight
{
public:
	PointLight(const Vector3& position, const Vector4& colour, float radius) {
		this->mPosition = position;
		this->mColour = colour;
		this->mRadius = radius;
		mName = "point";
	}

	~PointLight() {};
	Vector3 GetPosition() const { 
		return mPosition; 
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
protected:
	Vector3 mPosition;
	float mRadius;
};