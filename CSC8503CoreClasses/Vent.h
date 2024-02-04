#pragma once
#include "GameObject.h"

namespace NCL {
    namespace CSC8503 {
        class Vent : public GameObject {
        public:
            Vent();
            void ConnectVent(Vent* vent);
            bool IsOpen() { return mIsOpen; }
            void ToggleOpen() { mIsOpen = !mIsOpen; }
        protected:
            bool mIsOpen;
            Vent* mConnectedVent;
        };
    }
}

