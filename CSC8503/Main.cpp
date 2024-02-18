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

namespace{
    constexpr int SERVER_CHOICE = 1;
}

int main(){
    Window* w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);

    if (!w->HasInitialised()) {
        return -1;
    }


    auto* sceneManager = SceneManager::GetSceneManager();
    
    GameSceneManager* gm = nullptr;
    //erendgrmnc: make the bool below true for network test.
    bool isNetworkTestActive = false;

    w->ShowOSPointer(isNetworkTestActive);
    w->LockMouseToWindow(!isNetworkTestActive);
    
    if (isNetworkTestActive){
        gm = new DebugNetworkedGame();

        int choice = 0;
        std::cout << "--------Network Test ----------" << std::endl;
        std::cout <<"Enter '1' to start as server or '2' to start as client: ";
        std::cin >> choice;
        
        auto* networkedGame = (DebugNetworkedGame*)gm;
        if (choice == SERVER_CHOICE){
            networkedGame->StartAsServer();
        }
        else{
            networkedGame->StartAsClient(127, 0, 0, 1);
        }
    }
    else{
        gm = new GameSceneManager();
    }
    
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
        //gm->UpdateGame(dt);

        if (sceneManager->GetScenePushdownMachine() != nullptr) {
            sceneManager->GetScenePushdownMachine()->Update(dt);
        }
        if (sceneManager->GetCurrentScene() != nullptr) {
            sceneManager->GetCurrentScene()->UpdateGame(dt);
        }
    }
    Window::DestroyGameWindow();
}
