#include "SceneManager.h"
#include "PushdownMachine.h"
#include "DebugNetworkedGame.h"
#include "MainMenuScene.h"
#include "SceneStates.h"

SceneManager* NCL::CSC8503::SceneManager::instance = nullptr;

NCL::CSC8503::SceneManager::SceneManager() {
	currentScene = nullptr;
	InitScenes();
	InitPushdownMachine();
}

NCL::CSC8503::SceneManager::~SceneManager()
{
}

void NCL::CSC8503::SceneManager::InitScenes(){
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

void NCL::CSC8503::SceneManager::InitPushdownMachine()
{
	pushdownMachine = new PushdownMachine(new MainMenuSceneState());
}

void NCL::CSC8503::SceneManager::SetCurrentScene(Scenes scene){
	auto* nextScene = gameScenesMap[scene];
	currentScene = nextScene;
}

bool NCL::CSC8503::SceneManager::GetIsForceQuit()
{
	return isForceQuit;
}

void NCL::CSC8503::SceneManager::SetIsForceQuit(bool isForceQuit)
{
	this->isForceQuit = isForceQuit;
}

NCL::CSC8503::PushdownMachine* NCL::CSC8503::SceneManager::GetScenePushdownMachine()
{
	return pushdownMachine;
}

Scene* NCL::CSC8503::SceneManager::GetCurrentScene() {
	return currentScene;
}

SceneManager* NCL::CSC8503::SceneManager::GetSceneManager()
{
	if (instance == nullptr){
		instance = new SceneManager();
	}
	return instance;
}