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

	//InitIcons();

	if (mGameState == MainMenuState)
		DisplayMainMenu();

	if (mGameState == VictoryScreenState)
		DisplayVictory();

	if (mGameState == DefeatScreenState)
		DisplayDefeat();

	mLevelManager->Update(dt, mGameState == LevelState);
}

void GameSceneManager::InitCamera() {
	mLevelManager->GetGameWorld()->GetMainCamera().SetNearPlane(0.1f);
	mLevelManager->GetGameWorld()->GetMainCamera().SetFarPlane(500.0f);
	mLevelManager->GetGameWorld()->GetMainCamera().SetPitch(-15.0f);
	mLevelManager->GetGameWorld()->GetMainCamera().SetYaw(315.0f);
	mLevelManager->GetGameWorld()->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
}

//void GameSceneManager::InitIcons() {
//	UI::Icon mInventoryIcon1 = UI::AddIcon(Vector2(45, 90), 4.5, 8, mInventorySlotTex);
//	UI::Icon mInventoryIcon2 = UI::AddIcon(Vector2(50, 90), 4.5, 8, mInventorySlotTex);
//
//	UI::Icon mHighlightAwardIcon = UI::AddIcon(Vector2(3, 84), 4.5, 7, mHighlightAwardTex, false);
//	UI::Icon mLightOffIcon = UI::AddIcon(Vector2(8, 84), 4.5, 7, mLightOffTex, false);
//	UI::Icon mMakingNoiseIcon = UI::AddIcon(Vector2(13, 84), 4.5, 7, mMakingNoiseTex, false);
//	UI::Icon mSilentRunIcon = UI::AddIcon(Vector2(18, 84), 4.5, 7, mSilentRunTex, false);
//	UI::Icon mSlowDownIcon = UI::AddIcon(Vector2(3, 92), 4.5, 7, mSlowDownTex, false);
//	UI::Icon mStunIcon = UI::AddIcon(Vector2(8, 92), 4.5, 7, mStunTex, false);
//	UI::Icon mSwapPositionIcon = UI::AddIcon(Vector2(13, 92), 4.5, 7, mSwapPositionTex, false);
//
//	UI::Icon mSuspensionBarIcon = UI::AddIcon(Vector2(90, 16), 12, 75, mSuspensionBarTex);
//	UI::Icon mSuspensionIndicatorIcon = UI::AddIcon(Vector2(93, 86), 5, 5, mSuspensionIndicatorTex);
//}

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