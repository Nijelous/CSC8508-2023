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

#include "PushdownMachine.h"
#include "PushdownState.h"
#include "PushdownStates.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>
#include "DebugNetworkedGame.h"

int main() {
    Window* w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);

    if (!w->HasInitialised()) {
        return -1;
    }

    w->ShowOSPointer(false);
    w->LockMouseToWindow(true);

    GameManager* gm = nullptr;
    //erendgrmnc: make the bool below true for network test.
    bool isNetworkTestActive = false;
    bool isServer = false;
    if (isNetworkTestActive){
        gm = new DebugNetworkedGame();
        auto* networkedGame = (DebugNetworkedGame*)gm;
        if (isServer){
            networkedGame->StartAsServer();
        }
        else{
            networkedGame->StartAsClient(127, 0, 0, 1);
        }
    }
    else{
        gm = new GameManager();
    }
    PushdownMachine pushdownMachine(new MainMenu(gm));
    w->GetTimer().GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
    while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {
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
        pushdownMachine.Update(dt);
        gm->UpdateGame(dt);
    }
    Window::DestroyGameWindow();
}
