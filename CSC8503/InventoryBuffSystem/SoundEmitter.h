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
            SoundEmitter(int initCooldown, LocationBasedSuspicion* locationBasedSuspicionPTR);
            ~SoundEmitter();

            virtual void Update(float dt);

        protected:
            int mInitCooldown;
            LocationBasedSuspicion* mLocationBasedSuspicionPTR;
        };
    }
}
