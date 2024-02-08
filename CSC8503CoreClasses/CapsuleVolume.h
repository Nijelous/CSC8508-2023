#pragma once
#include "CollisionVolume.h"

namespace NCL {
    class CapsuleVolume : public CollisionVolume
    {
    public:
        CapsuleVolume(float halfHeight, float radius) {
            this->halfHeight    = halfHeight;
            this->radius        = radius;
            this->type          = VolumeType::Capsule;
            this->applyPhysics  = true;
        };
        ~CapsuleVolume() {

        }
        float GetRadius() const {
            return radius;
        }

        void SetRadius(float newRadius) {
            radius = newRadius;
        }

        void SetHalfHeight(float newHalfHeight) override {
            halfHeight = newHalfHeight;
        }

        float GetHalfHeight() const {
            return halfHeight;
        }

    protected:
        float radius;
        float halfHeight;
    };
}

