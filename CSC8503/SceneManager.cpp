#include "SceneManager.h"
#include "PushdownMachine.h"
#include "DebugNetworkedGame.h"
#include "MainMenuScene.h"
#include "SceneStates.h"

using namespace NCL::CSC8503;

SceneManager* SceneManager::instance = nullptr;

SceneManager::SceneManager() {
	currentScene = nullptr;
	InitScenes();
	InitPushdownMachine();
}

SceneManager::~SceneManager() {
}

void SceneManager::InitScenes() {
	auto* singlePlayerScene = new GameSceneManager();
	auto* mainMenuScene = new MainMenuScene();
	auto* multiplayerScene = new DebugNetworkedGame();
	
	gameScenesMap =
	{
		{Scenes::Singleplayer, (Scene*)singlePlayerScene},
		{Scenes::Multiplayer, (Scene*)multiplayerScene},
		{Scenes::MainMenu, (Scene*)mainMenuScene}
	};
}

void SceneManager::InitPushdownMachine() {
	pushdownMachine = new PushdownMachine(new MainMenuSceneState(currentScene));
}

void SceneManager::SetCurrentScene(Scenes scene) {
	auto* nextScene = gameScenesMap[scene];
	currentScene = nextScene;
}

bool SceneManager::GetIsForceQuit() {
	return isForceQuit;
}

void SceneManager::SetIsForceQuit(bool isForceQuit) {
	this->isForceQuit = isForceQuit;
}

PushdownMachine* SceneManager::GetScenePushdownMachine() {
	return pushdownMachine;
}

Scene* SceneManager::GetCurrentScene() {
	return currentScene;
}

SceneManager* SceneManager::GetSceneManager() {
	if (instance == nullptr){
		instance = new SceneManager();
	}
	return instance;
}