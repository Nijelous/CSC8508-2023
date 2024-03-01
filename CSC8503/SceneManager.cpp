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

	mCurrentSceneType = Scenes::MainMenu;

	gameScenesMap =
	{
		{Scenes::Singleplayer, (Scene*)singlePlayerScene},
		{Scenes::Multiplayer, (Scene*)multiplayerScene},
		{Scenes::MainMenu, (Scene*)mainMenuScene}
	};
}

void SceneManager::InitPushdownMachine() {
	pushdownMachine = new PushdownMachine(new MainMenuSceneState());
}

void SceneManager::SetCurrentScene(Scenes scene) {
	mIsInSingleplayer = scene == Scenes::Singleplayer;
	auto* nextScene = gameScenesMap[scene];
	currentScene = nextScene;
	mCurrentSceneType = scene;
}

bool SceneManager::GetIsForceQuit() {
	return isForceQuit;
}

bool SceneManager::IsInSingleplayer() const {
	return mIsInSingleplayer;
}

const bool SceneManager::IsServer() const {
	auto* networkGame = (DebugNetworkedGame*)(currentScene);

	if (networkGame->GetServer()) {
		return true;
	}
	return false;
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