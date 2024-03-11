#include <map>

#include "../PS5Core/PS5Controller.h"

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

            PushdownMachine* GetScenePushdownMachine();
            Scene* GetCurrentScene();
            static SceneManager* GetSceneManager();

        	static PS5::PS5Controller* GetPS5Controller() {
                return instance->mPS5Controller;
            }

            void SetPS5Controller(PS5::PS5Controller* ps5Controller) {
                instance->mPS5Controller = ps5Controller;
            }

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

            PS5::PS5Controller* mPS5Controller;
        };
    }
}
