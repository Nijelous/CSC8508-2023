#include "GameSceneManager.h"
#include "GameWorld.h"
#include "TextureLoader.h"

#include "PlayerObject.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "AnimationObject.h"

#include <irrKlang.h>

using namespace NCL;
using namespace CSC8503;
using namespace irrklang;

namespace {
	constexpr float PLAYER_MESH_SIZE = 3.0f;
	constexpr float PLAYER_INVERSE_MASS = 0.5f;
}

GameSceneManager::GameSceneManager() : mController(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	mLevelManager = new LevelManager();

	mLevelManager->GetGameWorld()->GetMainCamera().SetController(mController);
	mController.MapAxis(0, "Sidestep");
	mController.MapAxis(1, "UpDown");
	mController.MapAxis(2, "Forward");
	mController.MapAxis(3, "XLook");
	mController.MapAxis(4, "YLook");

	InitCamera();
}

GameSceneManager::~GameSceneManager() {
	delete mLevelManager;
}

void GameSceneManager::UpdateGame(float dt) {
	mLevelManager->GetGameWorld()->GetMainCamera().UpdateCamera(dt);

	if (mGameState == MainMenuState)
		DisplayMainMenu();

	if (mGameState == VictoryScreenState)
		DisplayVictory();

	if (mGameState == DefeatScreenState)
		DisplayDefeat();

	mLevelManager->Update(dt, mGameState == LevelState);

	PlayerWonGame();
}

void GameSceneManager::InitCamera() {
	mLevelManager->GetGameWorld()->GetMainCamera().SetNearPlane(0.1f);
	mLevelManager->GetGameWorld()->GetMainCamera().SetFarPlane(500.0f);
	mLevelManager->GetGameWorld()->GetMainCamera().SetPitch(-15.0f);
	mLevelManager->GetGameWorld()->GetMainCamera().SetYaw(315.0f);
	mLevelManager->GetGameWorld()->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
}

void GameSceneManager::CreateLevel() {
	mLevelManager->LoadLevel(0, 0);
}

bool GameSceneManager::PlayerWonGame() {
	if (mLevelManager->CheckGameWon())
		return true;
	return false;
}

void GameSceneManager::DisplayMainMenu() {
	// to be replaced by proper UI
	mLevelManager->GetGameWorld()->ClearAndErase();
	mLevelManager->GetPhysics()->Clear();
	std::cout << "WELCOME" << std::endl;
	std::cout << "PRESS SPACE TO PLAY" << std::endl;
}

void GameSceneManager::DisplayVictory() {
	// to be replaced by proper UI
	mLevelManager->GetGameWorld()->ClearAndErase();
	mLevelManager->GetPhysics()->Clear();
	std::cout << "VICTORY!!!!!!!" << std::endl;
}

void GameSceneManager::DisplayDefeat() {
	// to be replaced by proper UI
	mLevelManager->GetGameWorld()->ClearAndErase();
	mLevelManager->GetPhysics()->Clear();
	std::cout << "defeat :(((((((" << std::endl;
}