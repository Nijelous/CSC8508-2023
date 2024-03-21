#pragma once

#include <memory>

#ifdef USEPROSPERO
#include "PS5Controller.h"
#endif


#include "../NCLCoreClasses/KeyboardMouseController.h"

namespace NCL{
    namespace CSC8503{
        class LevelManager;

        class Scene {
        public:
            Scene();
            ~Scene();

            virtual void UpdateGame(float dt);

            LevelManager* GetLevelManager();

            virtual void DrawCanvas();
        protected:
            LevelManager* mLevelManager;

            virtual void InitCamera();
#ifdef USEGL
            KeyboardMouseController mController;
#endif

#ifdef USEPROSPERO
        public:
            PS5::PS5Controller* GetPS5Controller() { return mController; }
        protected:
            PS5::PS5Controller* mController;
#endif
        };
    }
}
