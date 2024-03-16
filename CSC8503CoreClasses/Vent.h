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

            void SetIsOpen(bool isOpen, bool isSettedByServer);
            void Interact(NCL::CSC8503::InteractType interactType, GameObject* interactedObject = nullptr) override;
            virtual bool CanBeInteractedWith(InteractType interactType, GameObject* interactedObject = nullptr) override;
        protected:
            bool mIsOpen;
            Vent* mConnectedVent;

            void HandleItemUse(GameObject* userObj = nullptr);
            void HandlePlayerUse(GameObject* userObj = nullptr);
            void SyncVentStatusInMultiplayer() const;
            bool CanUseItem();
        };
    }
}

