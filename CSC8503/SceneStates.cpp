#include "SceneStates.h"
#include "Scene.h"

#include "DebugNetworkedGame.h"
#include "LevelManager.h"
#include "GameClient.h"
#include "GameSceneManager.h"
#include "GameServer.h"
#include "Window.h"
#include "SceneManager.h"
#include "MainMenuScene.h"

using namespace NCL::CSC8503;


void MainMenuSceneState::OnAwake() {
	Window* w = Window::GetWindow();

#ifdef USEGL
	w->ShowOSPointer(true);
#endif

	SceneManager* sceneManager = SceneManager::GetSceneManager();
	sceneManager->SetCurrentScene(Scenes::MainMenu);

	if (mMainMenuScene == nullptr) {
		mMainMenuScene = (MainMenuScene*)sceneManager->GetCurrentScene();
		mMainMenuScene->SetOpenPanel(MainMenuScene::LevelSelection);
		mMainMenuScene->SetLevelSelectionPanelState(MainMenuScene::Selection);
	}
} 

PushdownState::PushdownResult MainMenuSceneState::OnUpdate(float dt, PushdownState** newState) {
	MainMenuScene::MainMenuPanels currentMainMenuPanel = mMainMenuScene->GetOpenPanel();
	MainMenuScene::LevelSelectionPanelStates currentLevelSelectionPanelState = mMainMenuScene->GetLevelSelectionPanelState();

	if (currentLevelSelectionPanelState == MainMenuScene::StartSingleplayer || SceneManager::GetSceneManager()->GetControllerInterface()->GetSelectPressed()) {
		*newState = new SingleplayerState();
		return PushdownResult::Push;
	}
#ifdef USEGL
	if (currentMainMenuPanel == MainMenuScene::MultiplayerLobby) {
		*newState = new MultiplayerLobbyState();
		return PushdownResult::Push;
	}
#endif

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
	Window* w = Window::GetWindow();
	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);

	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::Singleplayer);
	SceneManager::GetSceneManager()->SetChangeSceneTrigger(Scenes::Singleplayer);
	GameSceneManager* gameScene = (GameSceneManager*)(SceneManager::GetSceneManager()->GetCurrentScene());
}
#ifdef USEGL
PushdownState::PushdownResult ServerState::OnUpdate(float dt, PushdownState** newState) {
	if (!mHostedSuccessfully) {
		*newState = new MainMenuSceneState();
		return PushdownResult::Push;
	}

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
	DebugNetworkedGame* server = (DebugNetworkedGame*)SceneManager::GetSceneManager()->GetScene(Scenes::Multiplayer);

	mHostedSuccessfully = server->StartAsServer(mPlayerName);
	if (mHostedSuccessfully) {
		Window* w = Window::GetWindow();
		w->ShowOSPointer(false);
		w->LockMouseToWindow(true);

		LevelManager::GetLevelManager()->SetGameState(GameStates::LevelState);
		sceneManager->SetIsServer(true);
		sceneManager->SetCurrentScene(Scenes::Multiplayer);
		sceneManager->SetChangeSceneTrigger(Scenes::Multiplayer);
	}}

PushdownState::PushdownResult ClientState::OnUpdate(float dt, PushdownState** newState) {
	if (!mIsClientConnected) {
		*newState = new MainMenuSceneState();
		return PushdownResult::Push;
	}
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
	DebugNetworkedGame* client = (DebugNetworkedGame*)SceneManager::GetSceneManager()->GetScene(Scenes::Multiplayer);

	mIsClientConnected = client->StartAsClient(ipToConnect[0], ipToConnect[1], ipToConnect[2], ipToConnect[3], mPlayerName);
	if (mIsClientConnected) {
		Window* w = Window::GetWindow();
		w->ShowOSPointer(false);
		w->LockMouseToWindow(true);

		sceneManager->SetCurrentScene(Scenes::Multiplayer);
		LevelManager::GetLevelManager()->SetGameState(GameStates::LevelState);
		sceneManager->SetIsServer(false);
	}
}

PushdownState::PushdownResult MultiplayerLobbyState::OnUpdate(float dt, PushdownState** newState) {
	MainMenuScene::MainMenuPanels currentMainMenuPanel = mMainMenuScene->GetOpenPanel();

	if (currentMainMenuPanel == MainMenuScene::LevelSelection) {
		*newState = new MainMenuSceneState();
		return PushdownResult::Push;
	}

	const MainMenuScene::MultiplayerLobbyPanelStates multiplayerLobbyState = mMainMenuScene->GetMultiplayerLobbyState();
	if (multiplayerLobbyState == MainMenuScene::MultiplayerLobbyPanelStates::StartAsClient) {
		*newState = new ClientState();
		ClientState* clientState = static_cast<ClientState*>(*newState);
		int* ip = mMainMenuScene->GetIpAdressToConnect();

		mMainMenuScene->SetMultiplayerLobbyState(MainMenuScene::Lobby);
		for (int i = 0; i < 4; i++) {
			clientState->ipToConnect[i] = ip[i];
		}
		clientState->mPlayerName = mMainMenuScene->GetPlayerName();

		return PushdownResult::Push;
	}
	if (multiplayerLobbyState == MainMenuScene::MultiplayerLobbyPanelStates::StartAsServer) {
		*newState = new ServerState();
		ServerState* serverState = static_cast<ServerState*>(*newState);
		serverState->mPlayerName = mMainMenuScene->GetPlayerName();
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
