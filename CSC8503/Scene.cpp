#include "Scene.h"

#include "LevelManager.h"

Scene::Scene() : mController(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()){
    mLevelManager = LevelManager::GetLevelManager();
    mLevelManager->GetGameWorld()->GetMainCamera().SetController(mController);
    mController.MapAxis(0, "Sidestep");
    mController.MapAxis(1, "UpDown");
    mController.MapAxis(2, "Forward");
    mController.MapAxis(3, "XLook");
    mController.MapAxis(4, "YLook");
}

Scene::~Scene(){

}

void Scene::UpdateGame(float dt){
    LevelManager::GetLevelManager()->Update(dt, true, false);
}

LevelManager* NCL::CSC8503::Scene::GetLevelManager()
{
    return mLevelManager;
}

void Scene::InitCamera(){
    mLevelManager->GetGameWorld()->GetMainCamera().SetNearPlane(0.1f);
    mLevelManager->GetGameWorld()->GetMainCamera().SetFarPlane(500.0f);
    mLevelManager->GetGameWorld()->GetMainCamera().SetPitch(-15.0f);
    mLevelManager->GetGameWorld()->GetMainCamera().SetYaw(315.0f);
    mLevelManager->GetGameWorld()->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
}
