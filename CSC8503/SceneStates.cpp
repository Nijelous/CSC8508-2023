#include "SceneStates.h"

#include "DebugNetworkedGame.h"
#include "Window.h"
#include "SceneManager.h"
#include "MainMenuScene.h"

void NCL::CSC8503::MainMenuSceneState::OnAwake(){	
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::MainMenu);
}

NCL::CSC8503::PushdownState::PushdownResult NCL::CSC8503::MainMenuSceneState::OnUpdate(float dt, PushdownState** newState)
{
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


NCL::CSC8503::PushdownState::PushdownResult NCL::CSC8503::SingleplayerState::OnUpdate(float dt, PushdownState** newState)
{
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		*newState = new MainMenuSceneState();
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void NCL::CSC8503::SingleplayerState::OnAwake() {
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::Singleplayer);
	auto* gameScene = (GameSceneManager*)(SceneManager::GetSceneManager()->GetCurrentScene());
}

NCL::CSC8503::PushdownState::PushdownResult NCL::CSC8503::ServerState::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		*newState = new MainMenuSceneState();
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void NCL::CSC8503::ServerState::OnAwake(){
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::Multiplayer);
	auto* server = (DebugNetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();
	server->StartAsServer();
}

NCL::CSC8503::PushdownState::PushdownResult NCL::CSC8503::ClientState::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		*newState = new MainMenuSceneState();
		return PushdownResult::Push;
	}
	return PushdownResult::NoChange;
}

void NCL::CSC8503::ClientState::OnAwake()
{
	SceneManager::GetSceneManager()->SetCurrentScene(Scenes::Multiplayer);
	auto* client = (DebugNetworkedGame*)SceneManager::GetSceneManager()->GetCurrentScene();
	client->StartAsClient(127, 0, 0, 1);
}