#include "Window.h"
#include "Windows.h"
#include "Psapi.h"

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

void InitialiseDebug(int* numProcessors, ULARGE_INTEGER* lastCPU, ULARGE_INTEGER* lastSysCPU, ULARGE_INTEGER* lastUserCPU, HANDLE* self) {
    SYSTEM_INFO sysInfo;
    FILETIME ftime, fsys, fuser;

    GetSystemInfo(&sysInfo);
    *numProcessors = sysInfo.dwNumberOfProcessors;

    GetSystemTimeAsFileTime(&ftime);
    memcpy(lastCPU, &ftime, sizeof(FILETIME));

    *self = GetCurrentProcess();
    GetProcessTimes(*self, &ftime, &ftime, &fsys, &fuser);
    memcpy(lastSysCPU, &fsys, sizeof(FILETIME));
    memcpy(lastUserCPU, &fuser, sizeof(FILETIME));
}

void PrintDebug(int* numProcessors, ULARGE_INTEGER* lastCPU, ULARGE_INTEGER* lastSysCPU, ULARGE_INTEGER* lastUserCPU, HANDLE* self, float dt) {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);

    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

    FILETIME ftime, fsys, fuser;
    ULARGE_INTEGER now, sys, user;
    double percent;
    GetSystemTimeAsFileTime(&ftime);
    memcpy(&now, &ftime, sizeof(FILETIME));

    *self = GetCurrentProcess();
    GetProcessTimes(*self, &ftime, &ftime, &fsys, &fuser);
    memcpy(&sys, &fsys, sizeof(FILETIME));
    memcpy(&user, &fuser, sizeof(FILETIME));
    percent = (sys.QuadPart - lastSysCPU->QuadPart) + (user.QuadPart - lastUserCPU->QuadPart);
    percent /= (now.QuadPart - lastCPU->QuadPart);
    percent /= *numProcessors;
    *lastCPU = now;
    *lastUserCPU = user;
    *lastSysCPU = sys;
    percent *= 100;

    Debug::Print(std::format("FPS: {:.0f}", 1000.0f / dt), Vector2(1, 6), Vector4(1, 1, 1, 1), 15.0f);
    Debug::Print(std::format("Physical Memory Used: {} MB", (pmc.WorkingSetSize) / 1048576), Vector2(1, 9), Vector4(1, 0, 0, 1), 15.0f);
    Debug::Print(std::format("Total Physical Memory: {} MB", statex.ullTotalPhys / 1048576), Vector2(1, 12), Vector4(1, 0, 0, 1), 15.0f);
    Debug::Print(std::format("Percentage Memory Used: {:.5f}%", (float)pmc.WorkingSetSize / statex.ullTotalPhys), Vector2(1, 15), Vector4(0, 1, 0, 1), 15.0f);
    Debug::Print(std::format("Percentage CPU Used: {:.2f}%", percent), Vector2(1, 18), Vector4(0, 0, 1, 1), 15.0f);
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
    bool showDebugMenu = false;
    ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
    int numProcessors;
    HANDLE self;
    InitialiseDebug(&numProcessors, &lastCPU, &lastSysCPU, &lastUserCPU, &self);

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

        if (Window::GetKeyboard()->KeyPressed(KeyCodes::F3)) {
            showDebugMenu = !showDebugMenu;
        }
        if (showDebugMenu) {
            PrintDebug(&numProcessors, &lastCPU, &lastSysCPU, &lastUserCPU, &self, dt);
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
