#include "MainMenuScene.h"

#include "Debug.h"

using namespace NCL::CSC8503;

MainMenuScene::MainMenuScene() {
    Scene::InitCamera();
}

MainMenuScene::~MainMenuScene() {
}

void MainMenuScene::UpdateGame(float dt) {

    Debug::Print("1-) Start Single Player", Vector2(30, 70), Debug::WHITE);
    Debug::Print("2-) Start Multi Player (Server)", Vector2(30, 75), Debug::WHITE);
    Debug::Print("3-) Start Multi Player (Client)", Vector2(30, 80), Debug::WHITE);

    Scene::UpdateGame(dt);
}
