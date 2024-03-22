#include "GameSceneManager.h"
#include "GameWorld.h"
#include "TextureLoader.h"

#include "PlayerObject.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "AnimationObject.h"
#include "LevelManager.h"
#include "Debug.h"

#include "SinglePlayerStates.h"

using namespace NCL;
using namespace CSC8503;

namespace {
	constexpr float PLAYER_MESH_SIZE = 3.0f;
	constexpr float PLAYER_INVERSE_MASS = 0.5f;
}

GameSceneManager* GameSceneManager::instance = nullptr;

GameSceneManager::GameSceneManager(bool isNetworkGame) {
	if (!isNetworkGame) {
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
		DisplayPauseScreen();


	if (mGameState == MainMenuState)
		DisplayMainMenu();
	if (mGameState == VictoryScreenState)
		DisplayVictory();
	if (mGameState == DefeatScreenState)
		DisplayDefeat();

	mLevelManager->Update(dt, mGameState == PlayingLevelState, mGameState == PauseScreenState);
}

void GameSceneManager::InitInGameMenuManager() {
	auto* mainMenu = new MainMenuPushdownState(this);
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
	std::random_device rd;
	std::mt19937 g(rd());
#ifdef USEGL
	mLevelManager->LoadLevel(1, g, 0);
#endif
#ifdef USEPROSPERO
	mLevelManager->LoadLevel(0, g, 0);
#endif
}

bool GameSceneManager::PlayerWonGame() {
	if (mLevelManager->CheckGameWon().mGameWon) {
		mPlayersFinalPoints = mLevelManager->CheckGameWon().mCurrentPoints;
		return true;
	}
	return false;
}

bool GameSceneManager::PlayerLostGame() {
	if (mLevelManager->CheckGameLost())
		return true;
	return false;
}

void GameSceneManager::DisplayMainMenu() {
	// to be replaced by proper UI
	Debug::Print("Welcome", Vector2(45, 50));
#ifdef USEGL
	Debug::Print("Press SPACE to continue", Vector2(30, 55));
#endif
#ifdef USEPROSEPERO
	Debug::Print("Press X to continue", Vector2(30, 55));
#endif
}

void GameSceneManager::DisplayVictory() {
	// to be replaced by proper UI
	Debug::Print("VICTORY", Vector2(45, 50));
	Debug::Print("YOU GOT " + std::to_string(mPlayersFinalPoints) + " POINTS", Vector2(38, 53));
}

void GameSceneManager::DisplayDefeat() {
	// to be replaced by proper UI
	Debug::Print("DEFEAT", Vector2(45, 50));
}

void GameSceneManager::DisplayPauseScreen() {
	Debug::Print("PAUSED", Vector2(47, 50));
	Debug::Print("Press E to exit", Vector2(40, 55));
	Debug::Print("Press R to restart", Vector2(37, 60));
}
