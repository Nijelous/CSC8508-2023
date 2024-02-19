#pragma once
#include "GameObject.h"
#include "Interactable.h"

namespace NCL {
    namespace CSC8503 {
        class Vent : public GameObject, public Interactable {
        public:
            Vent();
            void ConnectVent(Vent* vent);
            bool IsOpen() { return mIsOpen; }
            void ToggleOpen() { mIsOpen = !mIsOpen; }
            void HandleItemUse();
            void HandlePlayerUse();
            void Interact(NCL::CSC8503::InteractType interactType) override;
        protected:
            bool mIsOpen;
            Vent* mConnectedVent;
        };
    }
}

