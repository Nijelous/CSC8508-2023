#include "Door.h"
#include "../CSC8503/LevelManager.h"
#include "../CSC8503/DebugNetworkedGame.h"
#include "../CSC8503/SceneManager.h"

using namespace NCL::CSC8503;

void Door::CountDownTimer(float dt)
{
	mTimer = std::max(mTimer - dt, 0.0f);
}

void Door::SetIsOpen(bool toOpen) {
	if (toOpen == mIsOpen)
		return;

	auto* sceneManager = SceneManager::GetSceneManager();
	const bool isSingleplayer = sceneManager->IsInSingleplayer();
	if (!isSingleplayer)
	{
		DebugNetworkedGame* networkedGame = static_cast<DebugNetworkedGame*>(sceneManager->GetCurrentScene());
		const bool isServer = networkedGame->GetIsServer();
		if (!isServer)
			return;
	}

	if(toOpen) {
		Open();
	}
	else {
		Close();
	}
}

