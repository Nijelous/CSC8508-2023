#pragma once
#include "PointLight.h"

using namespace NCL;
using namespace Maths;

class Spotlight : public PointLight
{
public:
	Spotlight(const Vector3& position, const Vector4& colour, float radius, float angle)
		:PointLight(position, colour, radius)
	{
		this->angle = angle;
		dotProdMin = cosf((angle / 2) * (PI / 180));
		dimProdMin = cosf(((angle - 2) / 2) * (PI / 180));
		direction = Vector3(0, 0, -1);
		name = "spot";
	}
	~Spotlight(void) {};
	float GetAngle() const { return angle; }
	void SetAngle(float angle) { this->angle = angle; dotProdMin = cosf(angle / 2); }
	Vector3 GetDirection() const { return direction; }
	void SetDirection(const Vector3& dir) { direction = dir; }
	float GetDotProdMin() const { return dotProdMin; }
	float GetDimProdMin() const { return dimProdMin; }

protected:
	float angle;
	float dotProdMin;
	float dimProdMin;
	Vector3 direction;
};