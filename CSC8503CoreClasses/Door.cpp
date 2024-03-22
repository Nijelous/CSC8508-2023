#include "Door.h"
#include "../CSC8503/LevelManager.h"
#include "../CSC8503/DebugNetworkedGame.h"
#include "../CSC8503/SceneManager.h"
#include "RenderObject.h"

using namespace NCL::CSC8503;

void Door::Open() {
	GetTransform().SetPosition(GetTransform().GetPosition() + Vector3(0, 7.25, 0));
	SetNavMeshFlags(1);
#ifdef USEGL
	this->GetSoundObject()->TriggerSoundEvent();
#endif
	mTimer = initDoorTimer;
	mIsOpen = true;
}

void Door::Close() {
	GetTransform().SetPosition(GetTransform().GetPosition() + Vector3(0,-7.25,0));
	SetNavMeshFlags(2);
#ifdef USEGL
	this->GetSoundObject()->CloseDoorTriggered();
#endif
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
#ifdef USEGL
	if (!isSingleplayer)
	{
		DebugNetworkedGame* networkedGame = static_cast<DebugNetworkedGame*>(sceneManager->GetCurrentScene());
		const bool isServer = networkedGame->GetIsServer();
		if (!isServer)
			return;
	}
#endif

	if(toOpen) {
		Open();
	}
	else {
		Close();
	}
}
void Door::SetNavMeshFlags(int flag) {
	if (flag == 4) {
		mRenderObject->SetAlbedoTexture(LevelManager::GetLevelManager()->GetTexture("DoorLockedAlbedo"));
	}
	else {
		mRenderObject->SetAlbedoTexture(LevelManager::GetLevelManager()->GetTexture("DoorAlbedo"));
	}
	float* pos = new float[3] { mTransform.GetPosition().x, mTransform.GetPosition().y, mTransform.GetPosition().z };
	AABBVolume* volume = (AABBVolume*)mBoundingVolume;
	float* halfSize = new float[3] { volume->GetHalfDimensions().x, volume->GetHalfDimensions().y, volume->GetHalfDimensions().z };
	LevelManager::GetLevelManager()->LoadDoorInNavGrid(pos, halfSize, (PolyFlags)flag);
}
