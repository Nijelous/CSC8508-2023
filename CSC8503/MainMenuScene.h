#pragma once

#include "Scene.h"

struct ImFont;

namespace NCL {
    namespace CSC8503 {

        class MainMenuScene : Scene {
        public:
            MainMenuScene();
            ~MainMenuScene();
			
            void UpdateGame(float dt) override;

            void DrawCanvas() override;
        protected:

            enum MainMenuPanels {
	            LevelSelection,
                MultiplayerLobby
            };

            MainMenuPanels mCurrentOpenPanel;

            ImFont* mHeaderFont = nullptr;
            ImFont* mButtonFont = nullptr;

            std::map<MainMenuPanels, std::function<void()>> mPanelDrawFuncMap;

            void HandleMainMenuControls();
            void InitPanelDrawFuncMap();

            void DrawLevelSelectionPanel();
            void DrawMultiplayerLobby();
        };
    }
}
