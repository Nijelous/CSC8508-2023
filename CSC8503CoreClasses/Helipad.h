#pragma once
#include "GameObject.h"

namespace NCL {
    namespace CSC8503 {
        class Helipad : public GameObject {
        public:
            Helipad();

            virtual void OnCollisionBegin(GameObject* otherObject) override;

            virtual void OnCollisionEnd(GameObject* otherObject) override;

            std::tuple<bool, int> GetCollidingWithPlayer() { return std::tuple<bool, int>(mCollidingWithPlayer, mCollidingPlayerID); }
        protected:
            bool mCollidingWithPlayer;
            int mCollidingPlayerID;
        };
    }
}
