#pragma once
#include "PushDownState.h"

namespace NCL {
    namespace CSC8503 {
        class Scene;

        class MainMenuSceneState : public PushdownState {
        public:
            MainMenuSceneState() {}

            PushdownResult OnUpdate(float dt, PushdownState** newState) override;

            void OnAwake() override;
        };

        class SingleplayerState : public PushdownState {
        public:
            SingleplayerState() {}

            PushdownResult OnUpdate(float dt, PushdownState** newState) override;

            void OnAwake() override;
        };
#ifdef USEGL
        class ServerState : public PushdownState {
        public:
            ServerState() {}

            PushdownResult OnUpdate(float dt, PushdownState** newState) override;

            void OnAwake() override;
        };

        class ClientState : public PushdownState {
        public:
            ClientState() {}

            PushdownResult OnUpdate(float dt, PushdownState** newState) override;

            void OnAwake() override;
        };
#endif
    }
}