#pragma once
#include "GameObject.h"
#include "Vector3.h"
#include "StateMachine.h"
#include "PlayerInventory.h"
#include "../SuspicionSystem/LocationBasedSuspicion.h"

using namespace SuspicionSystem;

namespace NCL {
    namespace CSC8503 {
        class SoundEmitter : public GameObject {
        public:
            SoundEmitter() {};
            SoundEmitter(float initCooldown, LocationBasedSuspicion* locationBasedSuspicionPTR, const Vector3& position);
            ~SoundEmitter();

            virtual void UpdateObject(float dt) override;

        protected:
            float mCooldown;
            LocationBasedSuspicion* mLocationBasedSuspicionPTR;
        };
    }
}
