#include "MultiplayerStates.h"
#include "DebugNetworkedGame.h"
#include "GameClient.h"
#include "GameServer.h"
#include "SceneManager.h"
#include "SinglePlayerStates.h"
#include "LevelManager.h"
#include "Debug.h"

MultiplayerLobby::MultiplayerLobby(DebugNetworkedGame* currentGameState) {
	mGameSceneManager = currentGameState;
}

PushdownState::PushdownResult MultiplayerLobby::OnUpdate(float dt, PushdownState** newState) {
	const bool isServer = mGameSceneManager->GetServer() != nullptr;
	if (isServer) {
		Debug::Print(" Waiting for player to join ...", Vector2(5, 95), Debug::RED);
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::S)) {

			mGameSceneManager->SetIsGameStarted(true);
			*newState = new InitialisingMultiplayerLevel(mGameSceneManager);
			return PushdownResult::Push;
		}
	}
	else {
		bool isConnected = mGameSceneManager->GetClient()->GetIsConnected();
		if (isConnected) {
			Debug::ClearStringEntries();
			Debug::Print("Connected Successfully! Waiting for server to start...", Vector2(5, 95), Debug::RED, 15.f);
;			if (bool isGameStarted = mGameSceneManager->GetIsGameStarted()) {
				*newState = new InitialisingMultiplayerLevel(mGameSceneManager);
				return PushdownResult::Push;
			}
		}
		else {
			Debug::Print(" Trying To Connect .", Vector2(5, 60), Debug::RED);
			Debug::Print(" Press ESC to return back.", Vector2(5, 80), Debug::RED);
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
				LevelManager::GetLevelManager()->SetGameState(MenuState);
			}
		}

	}
	return PushdownResult::NoChange;
	
}

void MultiplayerLobby::OnAwake() {
	mGameSceneManager->SetIsGameFinished(false, -1);
	mGameSceneManager->SetIsGameStarted(false);
}

InitialisingMultiplayerLevel::InitialisingMultiplayerLevel(DebugNetworkedGame* currentGameState){
	mGameSceneManager = currentGameState;
	
}

PushdownState::PushdownResult InitialisingMultiplayerLevel::OnUpdate(float dt, PushdownState** newState) {
	*newState = new PlayingMultiplayerLevel(mGameSceneManager);
	return PushdownResult::Push;
}

void InitialisingMultiplayerLevel::OnAwake() {
	auto* levelManager = LevelManager::GetLevelManager();
	levelManager->GetRenderer()->SetIsGameStarted(true);
}

PushdownState::PushdownResult PlayingMultiplayerLevel::OnUpdate(float dt, PushdownState** newState) {
	if (mGameSceneManager->PlayerWonGame()) {
		*newState = new MultiplayerVictory(mGameSceneManager);
		return PushdownResult::Push;
	}
	if (mGameSceneManager->PlayerLostGame()) {
		*newState = new MultiplayerDefeat(mGameSceneManager);
		return PushdownResult::Push;
	}

	return PushdownResult::NoChange;
}

void PlayingMultiplayerLevel::OnAwake() {

}

PushdownState::PushdownResult MultiplayerVictory::OnUpdate(float dt, PushdownState** newState) {
	Debug::Print("You Win! :))))))", Vector2(25, 50), Debug::RED);
	Debug::Print("Press escape to return main menu", Vector2(25, 60), Debug::WHITE);
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		*newState = new MultiplayerLobby(mGameSceneManager);
		LevelManager::GetLevelManager()->SetGameState(MenuState);
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void MultiplayerVictory::OnAwake() {
	auto* levelManager = LevelManager::GetLevelManager();
	mGameSceneManager->ClearNetworkGame();
	levelManager->GetRenderer()->SetIsGameStarted(false);
}


MultiplayerDefeat::MultiplayerDefeat(DebugNetworkedGame* currentGameState) {
	mGameSceneManager = currentGameState;
}

PushdownState::PushdownResult MultiplayerDefeat::OnUpdate(float dt, PushdownState** newState) {
	Debug::Print("You lost! :(", Vector2(25, 50), Debug::RED);
	Debug::Print("Press escape to return main menu", Vector2(25, 60), Debug::WHITE);
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		*newState = new MultiplayerLobby(mGameSceneManager);
		LevelManager::GetLevelManager()->SetGameState(MenuState);
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void MultiplayerDefeat::OnAwake() {
	auto* levelManager = LevelManager::GetLevelManager();
	mGameSceneManager->ClearNetworkGame();
	levelManager->GetRenderer()->SetIsGameStarted(false);
}
