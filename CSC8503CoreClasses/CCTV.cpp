#include "CCTV.h"
#include "Debug.h"
#include "Plane.h"
#include "../CSC8503/LevelManager.h"
#include "../CSC8503/DebugNetworkedGame.h"
#include "../CSC8503/SceneManager.h"
#include "GameWorld.h"
#include "RenderObject.h"

using namespace NCL::CSC8503;
namespace {
	const float MIN_PLAYER_DIST = 125;
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
#ifdef USEGL
	else{
		DebugNetworkedGame* game = reinterpret_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
		if(game->GetLocalPlayer() !=nullptr && game->GetLocalPlayer()->GetRenderObject()!=nullptr)
			UpdateForPlayerObject(game->GetLocalPlayer(), dt);
	}
#endif
}

const bool CCTV::PlayerInRaycast(PlayerObject* mPlayerObject){
	const Vector3 thisPosition = GetTransform().GetPosition();
	const Vector3 playerObjPosition = mPlayerObject->GetTransform().GetPosition();
	const Vector3 dir = (playerObjPosition - thisPosition).Normalised();
	Ray ray = Ray(thisPosition, dir);
	RayCollision closestCollision;

	mWorld->Raycast(ray, closestCollision, true, this, true);
	const auto* objectHit = (GameObject*)closestCollision.node;

	if (objectHit == mPlayerObject)
		return true;

	return false;
}

const bool CCTV::CanSeePlayer(PlayerObject* mPlayerObject) {
	const Vector3 playerPos = mPlayerObject->GetTransform().GetPosition();
	const float playerCullSphereR = mPlayerObject->GetRenderObject()->GetCullSphereRadius();
	auto collisionVolume = mPlayerObject->GetBoundingVolume()->GetOffset();
	if (mPlayerObject->GetRenderObject() != nullptr &&
		mViewPyramid.SphereInsidePyramid(playerPos + collisionVolume, playerCullSphereR) &&
		PlayerInRaycast(mPlayerObject))
		return true;
	return false;
}

const void CCTV::OnPlayerSeen(PlayerObject* mPlayerObject){
	DrawDebugLines(true);
	const int playerID = mPlayerObject->GetPlayerID();
	if (!hadSeenPlayer[playerID])
		LevelManager::GetLevelManager()->GetSuspicionSystem()->GetLocalSuspicionMetre()->AddActiveLocalSusCause(LocalSuspicionMetre::cameraLOS, mPlayerObject->GetPlayerID());
	hadSeenPlayer[playerID] = true;
#ifdef USEGL
	this->GetSoundObject()->TriggerSoundEvent();
#endif
}

const void CCTV::OnPlayerNotSeen(PlayerObject* mPlayerObject){
	DrawDebugLines(false);
	const int playerID = mPlayerObject->GetPlayerID();
	if (hadSeenPlayer[playerID])
	{
		LevelManager::GetLevelManager()->GetSuspicionSystem()->GetLocalSuspicionMetre()->RemoveActiveLocalSusCause(LocalSuspicionMetre::cameraLOS, mPlayerObject->GetPlayerID());
		const float playerLocalSusVal = LevelManager::GetLevelManager()->GetSuspicionSystem()->GetLocalSuspicionMetre()->GetLocalSusMetreValue(mPlayerObject->GetPlayerID());
		const Vector3 thisPos = this->GetTransform().GetPosition();
		LevelManager::GetLevelManager()->GetSuspicionSystem()->GetLocationBasedSuspicion()->SetMinLocationSusAmount(thisPos,playerLocalSusVal);
	}
		
	hadSeenPlayer[playerID] = false;
#ifdef USEGL
	this->GetSoundObject()->SetNotTriggered();
#endif

}

void CCTV::AngleToNormalisedCoords(float angle, float& x, float& y){
	float radsAngle = Maths::DegreesToRadians(angle);

	x = sin(radsAngle);
	y = cos(radsAngle);

	float total = abs(x) + abs(y);
	x /= total;
	y /= total;
}

