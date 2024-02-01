#pragma once
#include "BaseLight.h"

using namespace NCL;
using namespace Maths;

class DirectionLight : public BaseLight
{
public:
	DirectionLight(Vector3 direction, const Vector4& colour)
	{
		direction.Normalise();
		this->direction = direction;
		this->colour = colour;
		name = "direction";
	}

	~DirectionLight(void) {};
	Vector3 GetDirection() const { return direction; }
	void SetDirection(const Vector3& val) { direction = val; }

protected:
	Vector3 direction;
};