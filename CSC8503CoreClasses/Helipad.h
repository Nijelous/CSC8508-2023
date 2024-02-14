#pragma once
#include "GameObject.h"

namespace NCL {
    namespace CSC8503 {
        class Helipad : public GameObject {
        public:
            Helipad();

            virtual void OnCollisionBegin(GameObject* otherObject) override;

            virtual void OnCollisionEnd(GameObject* otherObject) override;

            bool GetCollidingWithPlayer() { return mCollidingWithPlayer; }
        protected:
            bool mCollidingWithPlayer;
        };
    }
}
