#pragma once
#include "CollisionVolume.h"

namespace NCL {
    class CapsuleVolume : public CollisionVolume
    {
    public:
        CapsuleVolume(float halfHeight, float radius, const Maths::Vector3& offset = Maths::Vector3(0, 0, 0)) {
            this->halfHeight    = halfHeight;
            this->radius        = radius;
            this->offset = offset;
            this->type          = VolumeType::Capsule;
            this->applyPhysics  = true;
        };
        ~CapsuleVolume() {

        }
        float GetRadius() const {
            return radius;
        }

        void SetRadius(float newRadius) override {
            radius = newRadius;
        }

        void SetHalfHeight(float newHalfHeight) override {
            halfHeight = newHalfHeight;
        }

        float GetHalfHeight() const {
            return halfHeight;
        }

        Maths::Vector3 GetOffset() const override {
            return offset;
        }

    protected:
        float radius;
        float halfHeight;
        Maths::Vector3 offset;
    };
}

