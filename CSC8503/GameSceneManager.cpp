#include "GameSceneManager.h"
#include "GameWorld.h"
#include "TextureLoader.h"

#include "PlayerObject.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "AnimationObject.h"

#include <irrKlang.h>

#include "PushdownStates.h"

using namespace NCL;
using namespace CSC8503;
using namespace irrklang;

namespace {
	constexpr float PLAYER_MESH_SIZE = 3.0f;
	constexpr float PLAYER_INVERSE_MASS = 0.5f;
}

GameSceneManager::GameSceneManager(bool isNetworkGame) {
	if (!isNetworkGame){
		InitInGameMenuManager();
	}
	InitCamera();
}

GameSceneManager::~GameSceneManager() {
}

void GameSceneManager::UpdateGame(float dt) {

	if (mPushdownMachine != nullptr){
		mPushdownMachine->Update(dt);
	}

	mLevelManager->GetGameWorld()->GetMainCamera().UpdateCamera(dt);

	if (mGameState == MainMenuState)
		DisplayMainMenu();

	if (mGameState == VictoryScreenState)
		DisplayVictory();

	if (mGameState == DefeatScreenState)
		DisplayDefeat();

	mLevelManager->Update(dt, mGameState == LevelState);
}

void GameSceneManager::InitInGameMenuManager(){
	auto* mainMenu = new MainMenu(this);
	mPushdownMachine = new PushdownMachine(mainMenu);
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