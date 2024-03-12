#include "Window.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "GameServer.h"
#include "GameClient.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "GameSceneManager.h"

#include "NetworkedGame.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "DebugNetworkedGame.h"
#include "PushdownMachine.h"
#include "SceneManager.h"
using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>

namespace {
    constexpr int NETWORK_TEST_WIDTH = 800;
    constexpr int NETWORK_TEST_HEIGHT = 600;

    constexpr int GAME_WINDOW_WIDTH = 1280;
    constexpr int GAME_WINDOW_HEIGHT = 720;
}

int main(){
    bool isNetworkTestActive = false;

    float winWidth = isNetworkTestActive ? NETWORK_TEST_WIDTH : GAME_WINDOW_WIDTH;
    float winHeight = isNetworkTestActive ? NETWORK_TEST_HEIGHT : GAME_WINDOW_HEIGHT;
    bool isFullScreen = !isNetworkTestActive;

    Window* w = Window::CreateGameWindow("CSC8503 Game technology!", winWidth, winHeight, false);

    if (!w->HasInitialised()) {
        return -1;
    }

    auto* sceneManager = SceneManager::GetSceneManager();
    
    GameSceneManager* gm = nullptr;
    //erendgrmnc: make the bool below true for network test.

    w->ShowOSPointer(isNetworkTestActive);
    w->LockMouseToWindow(!isNetworkTestActive);

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
    Window::DestroyGameWindow();
}
