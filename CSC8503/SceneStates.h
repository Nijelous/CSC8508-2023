#pragma once
#include "PushDownState.h"

namespace NCL {
    namespace CSC8503 {
        class Scene;

        class MainMenuSceneState : public PushdownState {
        public:
            MainMenuSceneState(Scene* sceneManager) {
                mScene = sceneManager;
            }

            PushdownResult OnUpdate(float dt, PushdownState** newState) override;

            void OnAwake() override;

            Scene* mScene;
        };

        class SingleplayerState : public PushdownState {
        public:
            SingleplayerState(Scene* sceneManager) {
                mScene = sceneManager;
            }

            PushdownResult OnUpdate(float dt, PushdownState** newState) override;

            void OnAwake() override;

            Scene* mScene;
        };

        class ServerState : public PushdownState {
        public:
            ServerState(Scene* sceneManager) {
                mScene = sceneManager;
            }

            PushdownResult OnUpdate(float dt, PushdownState** newState) override;

            void OnAwake() override;

            Scene* mScene;
        };

        class ClientState : public PushdownState {
        public:
            ClientState(Scene* sceneManager) {
                mScene = sceneManager;
            }

            PushdownResult OnUpdate(float dt, PushdownState** newState) override;

            void OnAwake() override;

            Scene* mScene;
        };
    }
}