#pragma once
#include "BaseLight.h"

using namespace NCL;
using namespace Maths;

class PointLight : public BaseLight
{
public:
	PointLight(const Vector3& position, const Vector4& colour, float radius)
	{
		this->position = position;
		this->colour = colour;
		this->radius = radius;
		name = "point";
	}

	~PointLight(void) {};
	Vector3 GetPosition() const { return position; }
	void SetPosition(const Vector3& val) { position = val; }
	float GetRadius() const { return radius; }
	void SetRadius(float val) { radius = val; }
protected:
	Vector3 position;
	float radius;
};