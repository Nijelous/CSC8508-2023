#include "SinglePlayerStates.h"

#include "Window.h"

using namespace NCL::CSC8503;

// pause screen

PushdownState::PushdownResult Pause::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::P)) {
		*newState = new PlayingLevel(mGameSceneManager);
		return PushdownResult::Push;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::E)) {
		*newState = new MainMenu(mGameSceneManager);
		return PushdownResult::Push;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::R)) {
		*newState = new InitialisingLevel(mGameSceneManager);
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void Pause::OnAwake() {
	mGameSceneManager->SetPause();
}

// defeat screen

PushdownState::PushdownResult Defeat::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
		*newState = new MainMenu(mGameSceneManager);
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void Defeat::OnAwake() {
	mGameSceneManager->SetDefeat();
	LevelManager::GetLevelManager()->ClearLevel();
}

// Victory Screen

PushdownState::PushdownResult Victory::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
		*newState = new MainMenu(mGameSceneManager);
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void Victory::OnAwake() {
	mGameSceneManager->SetVictory();
	LevelManager::GetLevelManager()->ClearLevel();
}

// mid level

PushdownState::PushdownResult PlayingLevel::OnUpdate(float dt, PushdownState** newState)   {
	if (mGameSceneManager->PlayerWonGame()) {
		*newState = new Victory(mGameSceneManager);
		return PushdownResult::Push;
	}
	if (mGameSceneManager->PlayerLostGame()) {
		*newState = new Defeat(mGameSceneManager);
		return PushdownResult::Push;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::P) && LevelManager::GetLevelManager()->RoundHasStarted()) {
		*newState = new Pause(mGameSceneManager);
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void PlayingLevel::OnAwake() {

	mGameSceneManager->SetLevel();
}

// initialising level state

PushdownState::PushdownResult InitialisingLevel::OnUpdate(float dt, PushdownState** newState) {
	*newState = new PlayingLevel(mGameSceneManager);
	return PushdownResult::Push;
}

void InitialisingLevel::OnAwake() {
	auto* levelManager = LevelManager::GetLevelManager();
	levelManager->GetRenderer()->SetIsGameStarted(true);
	levelManager->ClearLevel();
	mGameSceneManager->SetInitLevel();
	mGameSceneManager->CreateLevel();
}

// main menu

PushdownState::PushdownResult MainMenu::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
		*newState = new InitialisingLevel(mGameSceneManager);
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void MainMenu::OnAwake() {
	auto* levelManager = LevelManager::GetLevelManager();
	levelManager->GetRenderer()->SetIsGameStarted(false);
	mGameSceneManager->SetMainMenu();
	levelManager->ClearLevel();
}