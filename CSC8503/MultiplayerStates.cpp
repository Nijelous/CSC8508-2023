#include "MultiplayerStates.h"
#include "DebugNetworkedGame.h"
#include "SceneManager.h"
#include "SinglePlayerStates.h"

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
		Debug::Print(" Waiting for server to start ...", Vector2(5, 95), Debug::RED);
	}
	mGameSceneManager->GetLevelManager()->GetRenderer()->Render();
	return PushdownResult::NoChange;
	
}

void MultiplayerLobby::OnAwake() {
	mGameSceneManager->GetLevelManager()->ClearLevel();
}

InitialisingMultiplayerLevel::InitialisingMultiplayerLevel(DebugNetworkedGame* currentGameState){
	mGameSceneManager = currentGameState;
}

PushdownState::PushdownResult InitialisingMultiplayerLevel::OnUpdate(float dt, PushdownState** newState) {
	*newState = new PlayingMultiplayerLevel(mGameSceneManager);
	return PushdownResult::Push;
}

void InitialisingMultiplayerLevel::OnAwake() {
	LevelManager::GetLevelManager()->ClearLevel();
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
	Debug::Print("You Win! :))))))", Vector2(50, 50), Debug::RED);
	Debug::Print("Press escape to return main menu", Vector2(50, 60), Debug::WHITE);
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		SceneManager::GetSceneManager()->SetCurrentScene(Scenes::MainMenu);
		return PushdownResult::Pop;
	}
	return PushdownResult::NoChange;
}

void MultiplayerVictory::OnAwake() {
	LevelManager::GetLevelManager()->ClearLevel();
}


MultiplayerDefeat::MultiplayerDefeat(DebugNetworkedGame* currentGameState) {
	mGameSceneManager = currentGameState;
}

PushdownState::PushdownResult MultiplayerDefeat::OnUpdate(float dt, PushdownState** newState) {
	Debug::Print("You lost! :(", Vector2(50, 50), Debug::RED);
	Debug::Print("Press escape to return main menu", Vector2(50, 60), Debug::WHITE);
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		SceneManager::GetSceneManager()->SetCurrentScene(Scenes::MainMenu);
		return PushdownResult::Pop;
	}
	return PushdownResult::NoChange;
}

void MultiplayerDefeat::OnAwake() {
	LevelManager::GetLevelManager()->ClearLevel();
}
