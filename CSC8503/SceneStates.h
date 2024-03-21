#pragma once
#include "PushDownState.h"

namespace NCL {
    namespace CSC8503 {
	    class MainMenuScene;
	    class Scene;

        class MainMenuSceneState : public PushdownState {
        public:
            MainMenuSceneState() {}

            PushdownResult OnUpdate(float dt, PushdownState** newState) override;

            void OnAwake() override;
            MainMenuScene* mMainMenuScene;
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

            bool mHostedSuccessfully;
            std::string mPlayerName;
        };

        class ClientState : public PushdownState {
        public:
            ClientState() {}

            PushdownResult OnUpdate(float dt, PushdownState** newState) override;

            void OnAwake() override;

            bool mIsClientConnected;
            int ipToConnect[4];
            std::string mPlayerName;
        };

        class MultiplayerLobbyState : public PushdownState {
        public:
            MultiplayerLobbyState(){}
            PushdownResult OnUpdate(float dt, PushdownState** pushFunc) override;
            void OnAwake() override;

            MainMenuScene* mMainMenuScene;
        };
#endif
    }
}