#pragma once
#include "Vector4.h"
#include "Vector3.h"

class BaseLight {
public:
	Light() {};
	virtual ~Light() = 0 {};
	Vector4 GetColour() const { return colour; }
	void SetColour(const Vector4& val) { colour = val; }
	std::string GetName() const { return name; }

protected:
	Vector4 colour;
	Vector4 intensity;
	std::string name;
};
