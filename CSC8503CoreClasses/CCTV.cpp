#include "CCTV.h"
#include "Debug.h"
#include "Plane.h"
#include "../CSC8503/LevelManager.h"
#include "../CSC8503/DebugNetworkedGame.h"
#include "../CSC8503/SceneManager.h"

using namespace NCL::CSC8503;

namespace{
}

void CCTV::DrawDebugLines(const bool canSeePlayer){
	Vector4 DebugColour;
	if (canSeePlayer)
		DebugColour = Debug::RED;
	else
		DebugColour = Debug::GREEN;

	Vector3 baseCorners[4];
	for (int i = 0; i < 4; i++)
	{
		baseCorners[i] = Pyramid::GetBaseCorner(mBase, mBaseL, i);
		Debug::DrawLine(GetTransform().GetPosition(), baseCorners[i], DebugColour);
		if (i > 0)
			Debug::DrawLine(baseCorners[i], baseCorners[i - 1], DebugColour);
	}
	Debug::DrawLine(baseCorners[0], baseCorners[3], DebugColour);
}

void CCTV::UpdateObject(float dt) {
	//if SinglePlayer
	if (mPlayerObject){
		if (CanSeePlayer(mPlayerObject))
			OnPlayerSeen(mPlayerObject);
		else
			OnPlayerNotSeen(mPlayerObject);
	}

	//if Multiplayer
	if(mServerPlayersPtr){
		for (auto entry : *mServerPlayersPtr)
		{
			DebugNetworkedGame* game = reinterpret_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
			
			if (game->GetLocalPlayer() != entry.second)
				continue;

			if (CanSeePlayer(entry.second))
				OnPlayerSeen(entry.second);
			else
				OnPlayerNotSeen(entry.second);
		}
	}
}

bool CCTV::CanSeePlayer(PlayerObject* mPlayerObject){
	if(mPlayerObject->GetRenderObject())
		return mViewPyramid.SphereInsidePyramid(mPlayerObject->GetTransform().GetPosition(), mPlayerObject->GetRenderObject()->GetCullSphereRadius());
	return false;
}

void CCTV::OnPlayerSeen(PlayerObject* mPlayerObject){
	DrawDebugLines(true);
	const int playerID = mPlayerObject->GetPlayerID();
	if (!hadSeenPlayer[playerID])
		LevelManager::GetLevelManager()->GetSuspicionSystem()->GetLocalSuspicionMetre()->AddActiveLocalSusCause(LocalSuspicionMetre::cameraLOS, mPlayerObject->GetPlayerID());
	hadSeenPlayer[playerID] = true;
}

void CCTV::OnPlayerNotSeen(PlayerObject* mPlayerObject){
	DrawDebugLines(false);
	const int playerID = mPlayerObject->GetPlayerID();
	if (hadSeenPlayer[playerID])
		LevelManager::GetLevelManager()->GetSuspicionSystem()->GetLocalSuspicionMetre()->RemoveActiveLocalSusCause(LocalSuspicionMetre::cameraLOS, mPlayerObject->GetPlayerID());
	hadSeenPlayer[playerID] = false;
}

