#pragma once

#include "PushdownMachine.h"
#include "PushdownState.h"
#include "GameSceneManager.h"

#include <chrono>
#include <thread>

using namespace NCL;
using namespace CSC8503;

class Defeat : public PushdownState {
public:
	Defeat(GameSceneManager* g) {
		this->game = g;
	}
private:
	PushdownResult OnUpdate(float dt, PushdownState** newState)  override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		game->SetDefeat();
	}

	GameSceneManager* game;
};

class Victory : public PushdownState {
public:
	Victory(GameSceneManager* g) {
		this->game = g;
	}
private:
	PushdownResult OnUpdate(float dt, PushdownState** newState)  override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		game->SetVictory();
	}

	GameSceneManager* game;
};

class PlayingLevel : public PushdownState {
public:
	PlayingLevel(GameSceneManager* g) {
		this->game = g;
	}
private:
	PushdownResult OnUpdate(float dt, PushdownState** newState)  override {
		if (game->PlayerWonGame()) {
			*newState = new Victory(game);
			return PushdownResult::Push;
		}
		if (game->PLayerLostGame()) {
			*newState = new Defeat(game);
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		game->SetLevel();
		game->CreateLevel();
	}

	GameSceneManager* game;
};

class MainMenu : public PushdownState {
public:
	MainMenu(GameSceneManager* g) {
		this->game = g;
	}

private:
	PushdownResult OnUpdate(float dt, PushdownState** newState)  override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
			*newState = new PlayingLevel(game);
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		game->SetMainMenu();
	}

	GameSceneManager* game;
};