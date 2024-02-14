#pragma once

#include "Scene.h"

namespace NCL {
    namespace CSC8503 {
        class MainMenuScene : Scene {
        public:
            MainMenuScene();
            ~MainMenuScene();
			
            void UpdateGame(float dt) override;
        protected:
            
            void HandleMainMenuControls();
        };
    }
}