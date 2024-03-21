#include <map>

#include "ControllerInterface.h"

namespace NCL {
    namespace CSC8503 {
		
        class Scene;
        class PushdownMachine;
		
        enum Scenes {
            MainMenu = 0,
            Singleplayer = 1,
            Multiplayer = 2
        };
        
        class SceneManager {
        public:

            void InitScenes();
            void InitPushdownMachine();
            void SetCurrentScene(Scenes scene);
			
            bool GetIsForceQuit();
            bool IsInSingleplayer() const;
            const bool IsServer() const;
            const bool IsClient() const;
            void SetIsForceQuit(bool isForceQuit);
            void SetIsServer(bool isServer);
            void SetChangeSceneTrigger(Scenes scene);
            
            PushdownMachine* GetScenePushdownMachine();
            Scene* GetCurrentScene();
            Scene* GetScene(Scenes sceneType);
            Scenes GetCurrentSceneType() const;
            static SceneManager* GetSceneManager();

            ControllerInterface* GetControllerInterface() const { return mControllerInterface; }

        protected:
            bool isForceQuit = false;
            bool mIsInSingleplayer = false;
            bool mIsServer = false;
            SceneManager();
            ~SceneManager();

            static SceneManager* instance;

            Scenes mCurrentSceneType;
            Scene* currentScene = nullptr;
            PushdownMachine* pushdownMachine = nullptr;

            std::map<Scenes, Scene*> gameScenesMap;

            ControllerInterface* mControllerInterface;
        };
    }
}
