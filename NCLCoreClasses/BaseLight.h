#pragma once
#include "Vector4.h"
#include "Vector3.h"

using namespace NCL;
using namespace Maths;

class Light {
public:

	enum Type {
		Spot,
		Point,
		Direction
	};

	virtual ~Light() {};
	Vector4 GetColour() const { 
		return mColour; 
	}

	const float* GetColourAddress() const {
		const float *address = &mColour.x;
		return address;
	}


	void SetColour(const Vector3& val) { 
		mColour = val; 
	}
	Type GetType() const { 
		return mType; 
	}

protected:
	Vector3 mColour;
	Type mType;
};
