#pragma once

#include <imgui/imgui.h>

#include "Scene.h"

struct ImFont;

namespace NCL {
    namespace CSC8503 {

        class MainMenuScene : Scene {
        public:
            enum MainMenuPanels {
                LevelSelection,
                MultiplayerLobby
            };

            enum LevelSelectionPanelStates {
                StartSingleplayer,
                Selection
            };

            enum MultiplayerLobbyPanelStates {
	            StartAsClient,
                StartAsServer,
                Lobby
            };

            MainMenuScene();
            ~MainMenuScene();
			
            void UpdateGame(float dt) override;

            void DrawCanvas() override;

            void SetOpenPanel(MainMenuPanels panel);

            MainMenuPanels GetOpenPanel() const;
            MultiplayerLobbyPanelStates GetMultiplayerLobbyState() const;
            void SetMultiplayerLobbyState(MultiplayerLobbyPanelStates state);

            LevelSelectionPanelStates GetLevelSelectionPanelState() const;
            void SetLevelSelectionPanelState(LevelSelectionPanelStates state);

        	int* GetIpAdressToConnect();
            const std::string& GetPlayerName() const;
        protected:

            bool mIsMultiplayerLobbyOnHost;

            LevelSelectionPanelStates mLevelSelectionState;

            MainMenuPanels mCurrentOpenPanel;
            MultiplayerLobbyPanelStates mMultiplayerLobbyState;
            std::string mPlayerName;

        	char mIpAdressInputBuffer[16] = "";
        	char mNameInputBuffer[30] = "";

            int mIpAddress[4];

            ImFont* mHeaderFont = nullptr;
            ImFont* mButtonFont = nullptr;
            ImFont* mInputFont = nullptr;

            std::map<MainMenuPanels, std::function<void()>> mPanelDrawFuncMap;

            void HandleMainMenuControls();
            void InitPanelDrawFuncMap();

            void DrawLevelSelectionPanel();
            void DrawMultiplayerLobby();

            void InitIpAddress();
            void TranslateIpAddress();
            void TranslatePlayerName();
        };
    }
}
