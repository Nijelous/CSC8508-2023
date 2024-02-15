#include "SinglePlayerStates.h"

#include "Window.h"

using namespace NCL::CSC8503;

// defeat screen

PushdownState::PushdownResult Defeat::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void Defeat::OnAwake() {
	mGameSceneManager->SetDefeat();
}

// Victory Screen

PushdownState::PushdownResult Victory::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void Victory::OnAwake() {
	mGameSceneManager->SetVictory();
}

// mid level

PushdownState::PushdownResult PlayingLevel::OnUpdate(float dt, PushdownState** newState)   {
	if (mGameSceneManager->PlayerWonGame()) {
		*newState = new Victory(mGameSceneManager);
		return PushdownResult::Push;
	}
	if (mGameSceneManager->PLayerLostGame()) {
		*newState = new Defeat(mGameSceneManager);
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void PlayingLevel::OnAwake() {
	mGameSceneManager->SetLevel();
	mGameSceneManager->CreateLevel();
}

// main menu

PushdownState::PushdownResult MainMenu::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
		*newState = new PlayingLevel(mGameSceneManager);
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void MainMenu::OnAwake() {
	mGameSceneManager->SetMainMenu();
}