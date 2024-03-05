#include "SceneStates.h"
#include "Scene.h"

#include "DebugNetworkedGame.h"
#include "GameClient.h"
#include "GameServer.h"
#include "Window.h"
#include "SceneManager.h"
#include "MainMenuScene.h"

using namespace NCL::CSC8503;

void MainMenuSceneState::OnAwake() {	
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::MainMenu);
}

PushdownState::PushdownResult MainMenuSceneState::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1)) {
		*newState = new SingleplayerState();
		return PushdownResult::Push;
	}
	
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2)) {
		*newState = new ServerState();
		return PushdownResult::Push;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM3)) {
		*newState = new ClientState();
		return PushdownResult::Push;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		SceneManager::GetSceneManager()->SetIsForceQuit(true);
		return PushdownResult::Pop;
	}
	return PushdownResult::NoChange;
}

PushdownState::PushdownResult SingleplayerState::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE) && LevelManager::GetLevelManager()->GetGameState() == MenuState) {
		*newState = new MainMenuSceneState();
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void SingleplayerState::OnAwake() {
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::Singleplayer);
	auto* gameScene = (GameSceneManager*)(SceneManager::GetSceneManager()->GetCurrentScene());
}

PushdownState::PushdownResult ServerState::OnUpdate(float dt, PushdownState** newState) {
	GameStates currentState = LevelManager::GetLevelManager()->GetGameState();
	bool isInMenuState = currentState == MenuState;
	if (isInMenuState) {
		*newState = new MainMenuSceneState();
		auto* networkGame = (DebugNetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();
		if (networkGame->GetIsServer()) {
			auto* server = networkGame->GetServer();
			server->Shutdown();
		}
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void ServerState::OnAwake(){
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::Multiplayer);
	SceneManager::GetSceneManager()->SetIsServer(true);
	auto* server = (DebugNetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();
	LevelManager::GetLevelManager()->SetGameState(GameStates::LevelState);
	server->StartAsServer();
}

PushdownState::PushdownResult ClientState::OnUpdate(float dt, PushdownState** newState) {
	GameStates currentState = LevelManager::GetLevelManager()->GetGameState();
	bool isInMenuState = currentState == MenuState;
	if (isInMenuState) {
		*newState = new MainMenuSceneState();
		DebugNetworkedGame* networkGame = (DebugNetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();
		if (!networkGame->GetIsServer()) {
			GameClient* client = networkGame->GetClient();
			client->Disconnect();
		}
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void ClientState::OnAwake() {
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::Multiplayer);
	SceneManager::GetSceneManager()->SetIsServer(false);
	LevelManager::GetLevelManager()->SetGameState(GameStates::LevelState);
	DebugNetworkedGame* client = (DebugNetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();
	//client->StartAsClient(10,58,221,142);
	//Localhost IP
	client->StartAsClient(127, 0, 0, 1);
}