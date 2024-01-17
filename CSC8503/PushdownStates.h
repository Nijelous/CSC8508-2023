#pragma once

#include "PushdownMachine.h"
#include "PushdownState.h"
#include "LevelGenerator.h"

#include <chrono>
#include <thread>

using namespace NCL;
using namespace CSC8503;

class WinScreen : public PushdownState {
public:
	WinScreen(LevelGenerator* g) {
		this->game = g;
	}

private:
	PushdownResult OnUpdate(float dt, PushdownState** newState)  override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		game->SetToWinScreen();
	}

	LevelGenerator* game;
};

class LoseScreen : public PushdownState {
public:
	LoseScreen(LevelGenerator* g) {
		this->game = g;
	}

private:
	PushdownResult OnUpdate(float dt, PushdownState** newState)  override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		game->SetToLoseScreen();
	}

	LevelGenerator* game;
};

class NetworkingLevel : public PushdownState {
public:
	NetworkingLevel(LevelGenerator* g) {
		this->game = g;
	}

private:
	PushdownResult OnUpdate(float dt, PushdownState** newState)  override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::BACK)) {
			*newState = new WinScreen(game);
			return PushdownResult::Push;
		}
		if ((game->GetItemsLeft() == 0) && (game->PlayerWin())) {
			*newState = new WinScreen(game);
			return PushdownResult::Push;
		}
		if ((game->GetItemsLeft() == 0) && (!game->PlayerWin())) {
			*newState = new LoseScreen(game);
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		game->GenerateNetworkingLevel();
	}

	LevelGenerator* game;
};

class NetworkingMenu : public PushdownState {
public:
	NetworkingMenu(LevelGenerator* g) {
		this->game = g;
	}
private:
	PushdownResult OnUpdate(float dt, PushdownState** newState)  override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE) || Window::GetKeyboard()->KeyPressed(KeyCodes::BACK)) {
			*newState = new NetworkingLevel(game);
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		game->SetToNetworkingMenu();
	}

	LevelGenerator* game;
};

class PathfindingLevel : public PushdownState {
public:
	PathfindingLevel(LevelGenerator* g) {
		this->game = g;
	}

private:
	PushdownResult OnUpdate(float dt, PushdownState** newState)  override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::BACK)) {
			*newState = new WinScreen(game);
			return PushdownResult::Push;
		}
		if (game->CheckPlayerHitEnemy()) {
			*newState = new LoseScreen(game);
			return PushdownResult::Push;
		}
		if (game->GetItemsLeft() == 0) {
			*newState = new WinScreen(game);
			return PushdownResult::Push;
		}if (game->GetTimer() <= 0.0f) {
			*newState = new LoseScreen(game);
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		game->GeneratePathfindingLevel();
	}

	LevelGenerator* game;
};

class PathfindingMenu : public PushdownState {
public:
	PathfindingMenu(LevelGenerator* g) {
		this->game = g;
	}
private:
	PushdownResult OnUpdate(float dt, PushdownState** newState)  override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE) || Window::GetKeyboard()->KeyPressed(KeyCodes::BACK)) {
			*newState = new PathfindingLevel(game);
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		game->SetToPathfindingMenu();
	}

	LevelGenerator* game;
};

class PhysicsLevel : public PushdownState {
public:
	PhysicsLevel(LevelGenerator* g) {
		this->game = g;
	}

private:
	PushdownResult OnUpdate(float dt, PushdownState** newState)  override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::BACK)) {
			*newState = new PathfindingMenu(game);
			return PushdownResult::Push;
		}
		if (game->GetItemsLeft() == 0) {
			*newState = new PathfindingMenu(game);
			return PushdownResult::Push;
		}
		if (game->GetTimer() <= 0.0f) {
			*newState = new LoseScreen(game);
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		game->GeneratePhysicsLevel();
	}

	LevelGenerator* game;
};

class PhysicsMenu : public PushdownState {
public:
	PhysicsMenu(LevelGenerator* g) {
		this->game = g;
	}
private:
	PushdownResult OnUpdate(float dt, PushdownState** newState)  override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE) || Window::GetKeyboard()->KeyPressed(KeyCodes::BACK)) {
			*newState = new PhysicsLevel(game);
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
 		game->SetToPhysicsMenu();
	}

	LevelGenerator* game;
};

class MainMenu : public PushdownState {
public:
	MainMenu(LevelGenerator* g) {
		this->game = g;
	}

private:
	PushdownResult OnUpdate(float dt, PushdownState** newState)  override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
			*newState = new PhysicsMenu(game);
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::BACK)) {
			*newState = new NetworkingMenu(game);
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		game->SetToMainMenu();
	}

	LevelGenerator* game;
};