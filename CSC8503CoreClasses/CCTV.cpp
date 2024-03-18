#include "CCTV.h"
#include "Debug.h"
#include "Plane.h"
#include "../CSC8503/LevelManager.h"
#include "../CSC8503/DebugNetworkedGame.h"
#include "../CSC8503/SceneManager.h"

using namespace NCL::CSC8503;
namespace {
	const float MIN_PLAYER_DIST = 150;
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
		baseCorners[i] = Pyramid::GetBaseCorner(GetBase(GetTransform().GetOrientation().ToEuler().y), mBaseL, i);
		Debug::DrawLine(GetTransform().GetPosition(), baseCorners[i], DebugColour);
		if (i > 0)
			Debug::DrawLine(baseCorners[i], baseCorners[i - 1], DebugColour);
	}
	Debug::DrawLine(baseCorners[0], baseCorners[3], DebugColour);
}

void CCTV::GenerateViewPyramid(){
	const Vector3 thisPos = GetTransform().GetPosition();

	mViewPyramid = Pyramid(thisPos, GetBase(GetTransform().GetOrientation().ToEuler().y), mBaseL);
}

Vector3 CCTV::GetBase(float angle){
	float x, z;
	AngleToNormalisedCoords(angle, x, z);
	return (GetTransform().GetPosition() + Vector3(x, 0, z) * (mBaseL * 1.25f))
		* Vector3(1,0,1) + Vector3(0,-4.5,0);
}

const void CCTV::UpdateForPlayerObject(PlayerObject* playerObjectPtr, const float dt){
	if ((playerObjectPtr->GetTransform().GetPosition() -
		GetTransform().GetPosition()).Length() > MIN_PLAYER_DIST)
		return;
	
	if (CanSeePlayer(playerObjectPtr))
		OnPlayerSeen(playerObjectPtr);
	else {
		OnPlayerNotSeen(playerObjectPtr);
		rotateAngle += dt;
		GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0,initAngle + 45 * cos(rotateAngle), 0));
		GenerateViewPyramid();
	}
}

void CCTV::UpdateObject(float dt) {
	//if SinglePlayer
	if (mPlayerObject){
		UpdateForPlayerObject(mPlayerObject,dt);
	}
	//if Multiplayer
	else{
		DebugNetworkedGame* game = reinterpret_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
		if(game->GetLocalPlayer() !=nullptr)
			UpdateForPlayerObject(game->GetLocalPlayer(), dt);
	}
}

bool CCTV::CanSeePlayer(PlayerObject* mPlayerObject) const{
	return mViewPyramid.SphereInsidePyramid(mPlayerObject->GetTransform().GetPosition(), mPlayerObject->GetRenderObject()->GetCullSphereRadius());
}

const void CCTV::OnPlayerSeen(PlayerObject* mPlayerObject){
	DrawDebugLines(true);
	const int playerID = mPlayerObject->GetPlayerID();
	if (!hadSeenPlayer[playerID])
		LevelManager::GetLevelManager()->GetSuspicionSystem()->GetLocalSuspicionMetre()->AddActiveLocalSusCause(LocalSuspicionMetre::cameraLOS, mPlayerObject->GetPlayerID());
	hadSeenPlayer[playerID] = true;
}

const void CCTV::OnPlayerNotSeen(PlayerObject* mPlayerObject){
	DrawDebugLines(false);
	const int playerID = mPlayerObject->GetPlayerID();
	if (hadSeenPlayer[playerID])
		LevelManager::GetLevelManager()->GetSuspicionSystem()->GetLocalSuspicionMetre()->RemoveActiveLocalSusCause(LocalSuspicionMetre::cameraLOS, mPlayerObject->GetPlayerID());
	hadSeenPlayer[playerID] = false;
}

void CCTV::AngleToNormalisedCoords(float angle, float& x, float& y) const{
	float radsAngle = Maths::DegreesToRadians(angle);

	x = sin(radsAngle);
	y = cos(radsAngle);

	float total = abs(x) + abs(y);
	x /= total;
	y /= total;
}

