#include "SceneManager.h"
#include "PushdownMachine.h"
#include "DebugNetworkedGame.h"
#include "MainMenuScene.h"
#include "SceneStates.h"
#include "GameSceneManager.h"

#ifdef USEGL

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

#ifdef USEGL

	GameTechRenderer* renderer = (GameTechRenderer*)(LevelManager::GetLevelManager()->GetRenderer());
	renderer->SetImguiCanvasFunc([this]
		{
			currentScene->DrawCanvas();
		});

#endif

	mIsInSingleplayer = scene == Scenes::Singleplayer;
	auto* nextScene = gameScenesMap[scene];
	currentScene = nextScene;
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

void SceneManager::SetChangeSceneTrigger(Scenes scene) {
	mCurrentSceneType = scene;
}

PushdownMachine* SceneManager::GetScenePushdownMachine() {
	return pushdownMachine;
}

Scene* SceneManager::GetCurrentScene() {
	return currentScene;
}

Scenes SceneManager::GetCurrentSceneType() const {
	return mCurrentSceneType;
}

SceneManager* SceneManager::GetSceneManager() {
	if (instance == nullptr) {
		instance = new SceneManager();
	}
	return instance;
}
#endif