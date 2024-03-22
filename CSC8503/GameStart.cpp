#include "AnimationSystem.h"
#include "../NCLCoreClasses/Window.h"

#include "../CSC8503CoreClasses/Debug.h"

#include "../CSC8503CoreClasses/StateMachine.h"
#include "../CSC8503CoreClasses/StateTransition.h"
#include "../CSC8503CoreClasses/State.h"

#include "../CSC8503CoreClasses/GameServer.h"
#include "../CSC8503CoreClasses/GameClient.h"

#include "../CSC8503CoreClasses/NavigationGrid.h"
#include "../CSC8503CoreClasses/NavigationMesh.h"

#include "GameSceneManager.h"

#include "NetworkedGame.h"

#include "../CSC8503CoreClasses/BehaviourNode.h"
#include "../CSC8503CoreClasses/BehaviourSelector.h"
#include "../CSC8503CoreClasses/BehaviourSequence.h"
#include "../CSC8503CoreClasses/BehaviourAction.h"
#include "DebugNetworkedGame.h"
#include "../CSC8503CoreClasses/PushdownMachine.h"
#include "SceneManager.h"

#ifdef USEPROSPERO
#include "../PS5Core/PS5Window.h"
#include "../PS5Core/PS5Controller.h"
#endif

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>

namespace {
#ifdef USEGL
    constexpr int NETWORK_TEST_WIDTH = 800;
    constexpr int NETWORK_TEST_HEIGHT = 600;

    constexpr int GAME_WINDOW_WIDTH = 1280;
    constexpr int GAME_WINDOW_HEIGHT = 720;

#endif
#ifdef USEPROSPERO
    constexpr int NETWORK_TEST_WIDTH = 1920;
    constexpr int NETWORK_TEST_HEIGHT = 1080;

    constexpr int GAME_WINDOW_WIDTH = 1920;
    constexpr int GAME_WINDOW_HEIGHT = 1080;
#endif
}

#ifdef USEPROSPERO
PS5::PS5Window* SetUpPS5Window(float winWidth, float winHeight, bool fullscreen){
	PS5::PS5Window* w = new PS5::PS5Window("Hello!", winWidth, winHeight);

    return w;
}
#endif

Window* SetUpPCWindow(float winWidth, float winHeight, bool fullscreen) {
    Window* w = Window::CreateGameWindow("CSC8503 Game technology!", winWidth, winHeight, false);
   
    return w;
}

void SetUpPCInputDevices(Window* w, bool isNetworkTestActive) {
    w->ShowOSPointer(isNetworkTestActive);
    w->LockMouseToWindow(!isNetworkTestActive);
}

int RunGame(){
    bool isNetworkTestActive = false;

    float winWidth = isNetworkTestActive ? NETWORK_TEST_WIDTH : GAME_WINDOW_WIDTH;
    float winHeight = isNetworkTestActive ? NETWORK_TEST_HEIGHT : GAME_WINDOW_HEIGHT;
    bool isFullScreen = !isNetworkTestActive;
    SceneManager* sceneManager = nullptr;
   
#ifdef USEGL
    Window* w = nullptr;
    w = SetUpPCWindow(winWidth, winHeight, isFullScreen);
    SetUpPCInputDevices(w, isNetworkTestActive);
    sceneManager = SceneManager::GetSceneManager();
#endif

#ifdef USEPROSPERO
    PS5::PS5Window* w = nullptr;
    w = SetUpPS5Window(winWidth, winHeight, isFullScreen);
    sceneManager = SceneManager::GetSceneManager();
    sceneManager->GetControllerInterface()->SetPS5Controller(w->GetController());
#endif

    //erendgrmnc: make the bool below true for network test.   

    w->GetTimer().GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
    while (w->UpdateWindow() && !sceneManager->GetIsForceQuit()) {
        float dt = w->GetTimer().GetTimeDeltaSeconds();
        if (dt > 0.1f) {
            std::cout << "Skipping large time delta" << std::endl;
            continue; //must have hit a breakpoint or something to have a 1 second frame time!
        }
        if (Window::GetKeyboard()->KeyPressed(KeyCodes::PRIOR)) {
            w->ShowConsole(true);
        }
        if (Window::GetKeyboard()->KeyPressed(KeyCodes::NEXT)) {
            w->ShowConsole(false);
        }

        if (Window::GetKeyboard()->KeyPressed(KeyCodes::T)) {
            w->SetWindowPosition(0, 0);
        }

        w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

        if (sceneManager->GetScenePushdownMachine() != nullptr) {
            sceneManager->GetScenePushdownMachine()->Update(dt);
        }
        if (sceneManager->GetCurrentScene() != nullptr) {
            sceneManager->GetCurrentScene()->UpdateGame(dt);
        }
    }

    //Note: B Schwarz - is this necessary/desirable for PS5?
    Window::DestroyGameWindow();

    return 0;
}
