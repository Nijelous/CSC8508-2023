#include "SceneStates.h"
#include "Scene.h"

#include "DebugNetworkedGame.h"
#include "GameClient.h"
#include "GameServer.h"
#include "Window.h"
#include "SceneManager.h"
#include "MainMenuScene.h"

using namespace NCL::CSC8503;

#ifdef USEGL

void MainMenuSceneState::OnAwake() {
	SceneManager* sceneManager = SceneManager::GetSceneManager();
	sceneManager->SetCurrentScene(Scenes::MainMenu);

	if (mMainMenuScene == nullptr) {
		mMainMenuScene = (MainMenuScene*)sceneManager->GetCurrentScene();
	}
} 

PushdownState::PushdownResult MainMenuSceneState::OnUpdate(float dt, PushdownState** newState) {
	MainMenuScene::MainMenuPanels currentMainMenuPanel = mMainMenuScene->GetOpenPanel();

	if (currentMainMenuPanel == MainMenuScene::MultiplayerLobby) {
		*newState = new MultiplayerLobbyState();
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
	SceneManager::GetSceneManager()->SetChangeSceneTrigger(Scenes::Singleplayer);
	GameSceneManager* gameScene = (GameSceneManager*)(SceneManager::GetSceneManager()->GetCurrentScene());
}

PushdownState::PushdownResult ServerState::OnUpdate(float dt, PushdownState** newState) {
	GameStates currentState = LevelManager::GetLevelManager()->GetGameState();
	bool isInMenuState = currentState == MenuState;
	if (isInMenuState) {
		*newState = new MainMenuSceneState();
		DebugNetworkedGame* networkGame = (DebugNetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();
		if (networkGame->GetIsServer()) {
			GameServer* server = networkGame->GetServer();
			server->Shutdown();
		}
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void ServerState::OnAwake(){
	SceneManager* sceneManager = SceneManager::GetSceneManager();
	sceneManager->SetCurrentScene(Scenes::Multiplayer);
	sceneManager->SetChangeSceneTrigger(Scenes::Multiplayer);
	sceneManager->SetIsServer(true);
	DebugNetworkedGame* server = (DebugNetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();
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
	SceneManager* sceneManager = SceneManager::GetSceneManager();
	sceneManager->SetCurrentScene(Scenes::Multiplayer);
	sceneManager->SetIsServer(false);
	LevelManager::GetLevelManager()->SetGameState(GameStates::LevelState);
	DebugNetworkedGame* client = (DebugNetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();

	client->StartAsClient(127, 0, 0, 1);
}

PushdownState::PushdownResult MultiplayerLobbyState::OnUpdate(float dt, PushdownState** newState) {
	MainMenuScene::MainMenuPanels currentMainMenuPanel = mMainMenuScene->GetOpenPanel();

	if (currentMainMenuPanel == MainMenuScene::LevelSelection) {
		*newState = new MainMenuSceneState();
		return PushdownResult::Push;
	}

	return PushdownResult::NoChange;
}

void MultiplayerLobbyState::OnAwake() {
	SceneManager* sceneManager = SceneManager::GetSceneManager();
	Scene* currentScene = sceneManager->GetCurrentScene();

	sceneManager->SetCurrentScene(Scenes::MainMenu);

	if (mMainMenuScene == nullptr) {
		mMainMenuScene = (MainMenuScene*)currentScene;
	}
}
#endif
