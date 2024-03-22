#pragma once

#include "PushdownMachine.h"
#include "PushdownState.h"
#include "GameSceneManager.h"

#include <chrono>
#include <thread>

using namespace NCL;
using namespace CSC8503;

class Pause : public PushdownState {
public:
	Pause(GameSceneManager* currentGameState) {
		mGameSceneManager = currentGameState;
	}

	PushdownResult OnUpdate(float dt, PushdownState** newState)  override;

	void OnAwake() override;

	GameSceneManager* mGameSceneManager;
};

class Defeat : public PushdownState {
public:
	Defeat(GameSceneManager* currentGameState) {
		mGameSceneManager = currentGameState;
	}

	PushdownResult OnUpdate(float dt, PushdownState** newState)  override;

	void OnAwake() override;

	GameSceneManager* mGameSceneManager;
};

class Victory : public PushdownState {
public:
	Victory(GameSceneManager* currentGameState) {
		mGameSceneManager = currentGameState;
	}

	PushdownResult OnUpdate(float dt, PushdownState** newState)  override;

	void OnAwake() override;

	GameSceneManager* mGameSceneManager;
};

class PlayingLevel : public PushdownState {
public:
	PlayingLevel(GameSceneManager* currentGameState) {
		mGameSceneManager = currentGameState;
	}

	PushdownResult OnUpdate(float dt, PushdownState** newState)  override;

	void OnAwake() override;

	GameSceneManager* mGameSceneManager;
};

class InitialisingLevel : public PushdownState {
public:
	InitialisingLevel(GameSceneManager* currentGameState) {
		mGameSceneManager = currentGameState;
	}

	PushdownResult OnUpdate(float dt, PushdownState** newState)  override;

	void OnAwake() override;

	GameSceneManager* mGameSceneManager;
};

class MainMenuPushdownState : public PushdownState {
public:
	MainMenuPushdownState(GameSceneManager* currentGameState) {
		mGameSceneManager = currentGameState;
	}

	PushdownResult OnUpdate(float dt, PushdownState** newState)  override;

	void OnAwake() override;

	GameSceneManager* mGameSceneManager;
};