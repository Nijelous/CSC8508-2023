#pragma once
#include "Vector4.h"
#include "Vector3.h"

using namespace NCL;
using namespace Maths;

class BaseLight {
public:
	BaseLight() {};
	virtual ~BaseLight() = 0 {};
	Vector4 GetColour() const { 
		return mColour; 
	}
	void SetColour(const Vector4& val) { 
		mColour = val; 
	}
	std::string GetName() const { 
		return mName; 
	}

protected:
	Vector4 mColour;
	Vector4 mIntensity;
	std::string mName;
};
