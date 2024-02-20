#include "SceneStates.h"
#include "Scene.h"

#include "DebugNetworkedGame.h"
#include "Window.h"
#include "SceneManager.h"
#include "MainMenuScene.h"

using namespace NCL::CSC8503;

void MainMenuSceneState::OnAwake() {	
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::MainMenu);
}

PushdownState::PushdownResult MainMenuSceneState::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1)) {
		*newState = new SingleplayerState(mScene);
		return PushdownResult::Push;
	}
	
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2)) {
		*newState = new ServerState(mScene);
		return PushdownResult::Push;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM3)) {
		*newState = new ClientState(mScene);
		return PushdownResult::Push;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		SceneManager::GetSceneManager()->SetIsForceQuit(true);
		return PushdownResult::Pop;
	}
	return PushdownResult::NoChange;
}

PushdownState::PushdownResult SingleplayerState::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		*newState = new MainMenuSceneState(mScene);
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void SingleplayerState::OnAwake() {
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::Singleplayer);
	auto* gameScene = (GameSceneManager*)(SceneManager::GetSceneManager()->GetCurrentScene());
}

PushdownState::PushdownResult ServerState::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		*newState = new MainMenuSceneState(mScene);
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void ServerState::OnAwake(){
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::Multiplayer);
	auto* server = (DebugNetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();
	server->StartAsServer();
}

PushdownState::PushdownResult ClientState::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		*newState = new MainMenuSceneState(mScene);
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void ClientState::OnAwake() {
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::Multiplayer);
	auto* client = (DebugNetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();
	client->StartAsClient(127, 0, 0, 1);
}