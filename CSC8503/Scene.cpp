#include "Scene.h"

#include "LevelManager.h"
#include "SceneManager.h"


#ifdef USEGL
Scene::Scene() : mController(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()){
    mLevelManager = LevelManager::GetLevelManager();
    mLevelManager->GetGameWorld()->GetMainCamera().SetController(mController);
    mController.MapAxis(0, "Sidestep");
    mController.MapAxis(1, "UpDown");
    mController.MapAxis(2, "Forward");
    mController.MapAxis(3, "XLook");
    mController.MapAxis(4, "YLook");
}
#endif

#ifdef USEPROSPERO
Scene::Scene() : mController(static_cast<NCL::PS5::PS5Window*>(Window::GetWindow())->GetController()) {
    mLevelManager = LevelManager::GetLevelManager();
    mController->MapAxis(0, "LeftX");
    mController->MapAxis(1, "LeftY");

    mController->MapAxis(2, "RightX");
    mController->MapAxis(3, "RightY");

    mController->MapAxis(4, "DX");
    mController->MapAxis(5, "DY");

    mController->MapButton(0, "Triangle");
    mController->MapButton(1, "Circle");
    mController->MapButton(2, "Cross");
    mController->MapButton(3, "Square");
    mController->MapButton(4, "L2");
    mController->MapButton(5, "R2");
    mController->MapButton(6, "L1");
    mController->MapButton(7, "R1");
    mController->MapButton(8, "L3");
    mController->MapButton(9, "R3");

    //These are the axis/button aliases the inbuilt camera class reads from:
    mController->MapAxis(2, "XLook");
    mController->MapAxis(3, "YLook");

    mController->MapAxis(0, "Sidestep");
    mController->MapAxis(1, "Forward");

    mController->MapButton(0, "Up");
    mController->MapButton(2, "Down");
    mLevelManager->GetGameWorld()->GetMainCamera().SetController(*static_cast<NCL::PS5::PS5Window*>(Window::GetWindow())->GetController());
}
#endif

Scene::~Scene(){

}

void Scene::UpdateGame(float dt){
    LevelManager::GetLevelManager()->Update(dt, false, false);
}

LevelManager* NCL::CSC8503::Scene::GetLevelManager() {
    return mLevelManager;
}

void Scene::DrawCanvas() {
}

void Scene::InitCamera(){
    mLevelManager->GetGameWorld()->GetMainCamera().SetNearPlane(0.1f);
    mLevelManager->GetGameWorld()->GetMainCamera().SetFarPlane(500.0f);
    mLevelManager->GetGameWorld()->GetMainCamera().SetPitch(-15.0f);
    mLevelManager->GetGameWorld()->GetMainCamera().SetYaw(315.0f);
    mLevelManager->GetGameWorld()->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
}

