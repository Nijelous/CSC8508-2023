#include "SceneManager.h"
#include "PushdownMachine.h"
#include "DebugNetworkedGame.h"
#include "MainMenuScene.h"
#include "SceneStates.h"
#include "GameSceneManager.h"



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
	GameSceneManager* singlePlayerScene = new GameSceneManager();
	MainMenuScene* mainMenuScene = new MainMenuScene();
#ifdef USEGL
	DebugNetworkedGame* multiplayerScene = new DebugNetworkedGame();
#endif

	mCurrentSceneType = Scenes::MainMenu;

	gameScenesMap =
	{
		{Scenes::Singleplayer, (Scene*)singlePlayerScene},
#ifdef USEGL
		{Scenes::Multiplayer, (Scene*)multiplayerScene},
#endif
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
	return mIsServer;
}

void SceneManager::SetIsForceQuit(bool isForceQuit) {
	this->isForceQuit = isForceQuit;
}

void SceneManager::SetIsServer(bool isServer) {
	mIsServer = isServer;
}

PushdownMachine* SceneManager::GetScenePushdownMachine() {
	return pushdownMachine;
}

Scene* SceneManager::GetCurrentScene() {
	return currentScene;
}

SceneManager* SceneManager::GetSceneManager() {
	if (instance == nullptr) {
		instance = new SceneManager();
	}
	return instance;
}
