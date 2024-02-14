#include <map>

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
            void SetIsForceQuit(bool isForceQuit);

            PushdownMachine* GetScenePushdownMachine();
            Scene* GetCurrentScene();
            static SceneManager* GetSceneManager();
        protected:
            bool isForceQuit = false;

            SceneManager();
            ~SceneManager();

            static SceneManager* instance;
			
            Scene* currentScene = nullptr;
            PushdownMachine* pushdownMachine = nullptr;

            std::map<Scenes, Scene*> gameScenesMap;
        };
    }
}
