#pragma once
#include "PushDownState.h"

namespace NCL {
    namespace CSC8503 {
        class MainMenuSceneState : public PushdownState {
            PushdownResult OnUpdate(float dt, PushdownState** newState) override;

            void OnAwake() override;
        };

        class SingleplayerState : public PushdownState {
            PushdownResult OnUpdate(float dt, PushdownState** newState) override;
            void OnAwake() override;
        };

        class ServerState : public PushdownState {
            PushdownResult OnUpdate(float dt, PushdownState** newState) override;
            void OnAwake() override;
        };

        class ClientState : public PushdownState {
            PushdownResult OnUpdate(float dt, PushdownState** newState) override;
            void OnAwake() override;
        };
    }
}