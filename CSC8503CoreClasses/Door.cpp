#include "Door.h"
#include "../CSC8503/LevelManager.h"
#include "../CSC8503/DebugNetworkedGame.h"
#include "../CSC8503/SceneManager.h"

using namespace NCL::CSC8503;

void Door::Open() {
	GetTransform().SetPosition(GetTransform().GetPosition() + Vector3(0, 7.25, 0));
	SetNavMeshFlags(1);
	this->GetSoundObject()->CloseDoorTriggered();
	mTimer = initDoorTimer;
	mIsOpen = true;
}

void Door::Close() {
	GetTransform().SetPosition(GetTransform().GetPosition() + Vector3(0,-7.25,0));
	SetNavMeshFlags(2);
	this->GetSoundObject()->CloseDoorTriggered();
	mTimer = -1;
	mIsOpen = false;
}

void Door::CountDownTimer(float dt)
{
	mTimer = std::max(mTimer - dt, 0.0f);
}

void Door::UpdateObject(float dt){
	if (mIsOpen && mTimer > 0)
		CountDownTimer(dt);

	if (mTimer == 0)
		SetIsOpen(false);
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
void Door::SetNavMeshFlags(int flag) {
	float* pos = new float[3] { mTransform.GetPosition().x, mTransform.GetPosition().y, mTransform.GetPosition().z };
	AABBVolume* volume = (AABBVolume*)mBoundingVolume;
	float* halfSize = new float[3] { volume->GetHalfDimensions().x, volume->GetHalfDimensions().y, volume->GetHalfDimensions().z };
	LevelManager::GetLevelManager()->LoadDoorInNavGrid(pos, halfSize, (PolyFlags)flag);
}
