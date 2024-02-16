#include "GameSceneManager.h"
#include "GameWorld.h"
#include "TextureLoader.h"

#include "PlayerObject.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "AnimationObject.h"

#include <irrKlang.h>

#include "SinglePlayerStates.h"

using namespace NCL;
using namespace CSC8503;
using namespace irrklang;

namespace {
	constexpr float PLAYER_MESH_SIZE = 3.0f;
	constexpr float PLAYER_INVERSE_MASS = 0.5f;
}

GameSceneManager* GameSceneManager::instance = nullptr;

GameSceneManager::GameSceneManager(bool isNetworkGame) {
	if (!isNetworkGame){
		InitInGameMenuManager();
	}
	InitCamera();
}

GameSceneManager::~GameSceneManager() {
}

GameSceneManager GameSceneManager::GetGameSceneManager() {
	if (instance == nullptr) {
		instance = new GameSceneManager();
	}
	return instance;
}

void GameSceneManager::UpdateGame(float dt) {

	if (mPushdownMachine != nullptr) {
		mPushdownMachine->Update(dt);
	}


	if (!(mGameState == PauseScreenState))
		mLevelManager->GetGameWorld()->GetMainCamera().UpdateCamera(dt);
	else
		Debug::Print("PAUSED", Vector2(50, 50));


	if (mGameState == MainMenuState)
		DisplayMainMenu();
	if (mGameState == VictoryScreenState)
		DisplayVictory();
	if (mGameState == DefeatScreenState)
		DisplayDefeat();

	mLevelManager->Update(dt, mGameState == PlayingLevelState, mGameState == PauseScreenState);

	PlayerWonGame();
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

bool GameSceneManager::PlayerWonGame() {
	if (mLevelManager->CheckGameWon().mGameWon) {
		if (mLevelManager->CheckGameWon().mCurrentPoints > 0)
			mPlayersFinalPoints = mLevelManager->CheckGameWon().mCurrentPoints;
		return true;
	}
	return false;
}

bool GameSceneManager::PLayerLostGame() {
	if (mLevelManager->CheckGameLost())
		return true;
	return false;
}

void GameSceneManager::DisplayMainMenu() {
	// to be replaced by proper UI
	mLevelManager->ClearLevel();
	Debug::Print("Welcome", Vector2(45, 50));
	Debug::Print("Press SPACE to continue", Vector2(30, 55));
}

void GameSceneManager::DisplayVictory() {
	// to be replaced by proper UI
	mLevelManager->ClearLevel();
	Debug::Print("VICTORY", Vector2(45, 50));
	Debug::Print("YOU GOT " + std::to_string(mPlayersFinalPoints) + " POINTS", Vector2(38, 53));
}

void GameSceneManager::DisplayDefeat() {
	// to be replaced by proper UI
	mLevelManager->ClearLevel();
	Debug::Print("DEFEAT", Vector2(45, 50));
}