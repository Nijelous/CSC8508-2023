//Created by Oliver Perrin
//Edited by Chris Chen and Eren Degirmenci 

#include "GuardObject.h"
#include "PlayerObject.h"
#include "Ray.h"
#include "Debug.h"
#include "PhysicsObject.h"
#include "BehaviourSelector.h"
#include "PlayerObject.h"
#include "../CSC8503/LevelManager.h"
#include "../Detour/Include/DetourNavMeshQuery.h"
#include "RecastBuilder.h"
#include "InteractableDoor.h"
#include "../CSC8503/SceneManager.h"
#include "../CSC8503/NetworkPlayer.h"
#include "PrisonDoor.h"
#include "../CSC8503/DebugNetworkedGame.h"

using namespace NCL;
using namespace CSC8503;

GuardObject::GuardObject(const std::string& objectName) {
	mName = objectName;
	mRootSequence = new BehaviourSequence("Root Sequence");
	mCanSeePlayer = false;
	mHasCaughtPlayer = false;
	mPlayerHasItems = true;
	mIsStunned = false;
	mDist = 0;
	mNextPoly = 0;
	mDoorRaycastInterval = RAYCAST_INTERVAL;
	mFumbleKeysCurrentTime = FUMBLE_KEYS_TIME;
	mPointTimer = POINTING_TIMER;
	mNearestSprintingPlayerDir = nullptr;
	mLastDist = 0;
	mDistCounter = 0;
	mDebugMode = false;

	SceneManager* sceneManager = SceneManager::GetSceneManager();

	const bool isInSinglePlayer = sceneManager->IsInSingleplayer();
	const bool isServer = sceneManager->IsServer();
	mIsBTWillBeExecuted = isInSinglePlayer || isServer;
	if (mIsBTWillBeExecuted) {
		BehaviourTree();
	}
}

GuardObject::~GuardObject() {
	delete mRootSequence;
	delete[] mNextPoly;
	delete[] mLastKnownPos;
	delete mNearestSprintingPlayerDir;
	delete mSightedDoor;
	delete mSightedPlayer;
	delete[] mNextPoly;
}

void GuardObject::UpdateObject(float dt) {

	if (!mIsStunned) {
		if (mIsBTWillBeExecuted) {
			RaycastToPlayer();
			DebugMode();
			GuardSpeedMultiplier();
			ExecuteBT();
			if (mDoorRaycastInterval <= 0) {
				mDoorRaycastInterval = RAYCAST_INTERVAL;
				CheckForDoors(dt);
			}
			else {
				mDoorRaycastInterval -= dt;
			}
		}
	}
	else {
		Debug::Print("Guard Is Stunned! ", Vector2(10, 40));
	}
	HandleAppliedBuffs(dt);
}

void GuardObject::RaycastToPlayer() {

	PlayerObject* playerToChase = GetPlayerToChase();
	if (playerToChase) {
		RayCollision closestCollision;
		Vector3 playerToChaseDir = (playerToChase->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Normalised();
		Ray r = Ray(this->GetTransform().GetPosition(), playerToChaseDir);
		if (LevelManager::GetLevelManager()->GetGameWorld()->Raycast(r, closestCollision, true, this, true)) {
			mSightedPlayer = (GameObject*)closestCollision.node;
			if (mDebugMode == true){ Debug::DrawLine(this->GetTransform().GetPosition(), closestCollision.collidedAt); }
			if (mSightedPlayer->GetCollisionLayer() == CollisionLayer::Player) {
				mPlayer = static_cast<PlayerObject*>(mSightedPlayer);
				mCanSeePlayer = true;
			}
			else {
				mCanSeePlayer = false;
				mSightedPlayer = nullptr;
			}
		}
		else {
			mCanSeePlayer = false;
		}
	}
	else {
		mCanSeePlayer = false;
		mSightedPlayer = nullptr;
		delete playerToChase;
	}
}

void GuardObject::DebugMode() {
	if (Window::GetKeyboard()->KeyDown(KeyCodes::F6)) {
		mDebugMode = !mDebugMode;
	}
}

void GuardObject::GuardSpeedMultiplier() {
	if (LevelManager::GetLevelManager()->GetSuspicionSystem()->GetGlobalSuspicionMetre()->GetGlobalSusMeter() > 50 && mCanSeePlayer == true) {
		mGuardSpeedMultiplier = 45;
		}
	else if (LevelManager::GetLevelManager()->GetSuspicionSystem()->GetGlobalSuspicionMetre()->GetGlobalSusMeter() > 50 || mCanSeePlayer == true) {
		mGuardSpeedMultiplier = 35;
	}
	else {
		mGuardSpeedMultiplier = 25;
	}
}

Vector3 GuardObject::GuardForwardVector() {
	Vector3 rightAxis = this->mTransform.GetMatrix().GetColumn(0);
	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	return fwdAxis;
}

float GuardObject::AngleFromFocalPoint(Vector3 direction) {
	Vector3 upVector = this->GetTransform().GetOrientation() * Vector3(0, 1, 0);
	Vector3 perpendicularVector = Vector3::Cross(GuardForwardVector(), upVector);
	float angle = Vector3::Dot(direction.Normalised(), perpendicularVector.Normalised());
	return angle;
}

void GuardObject::HandleAppliedBuffs(float dt) {
	std::vector<PlayerBuffs::buff> buffsToRemove;

	for (auto& appliedBuff : mAppliedBuffs) {
		if (appliedBuff.second <= 0.f) {
			RemoveBuffFromGuard(appliedBuff.first);
			buffsToRemove.push_back(appliedBuff.first);
		}
		else {
			appliedBuff.second -= dt;
		}
	}

	for (auto& toBeRemovedBuff : buffsToRemove) {
		mAppliedBuffs.erase(toBeRemovedBuff);
	}
}

PlayerObject* GuardObject::GetPlayerToChase() {
	float minDist = INT_MAX;
	PlayerObject* playerToChase = nullptr;

	for (PlayerObject* player : mPlayerList) {
		Vector3 dir = player->GetTransform().GetPosition() - this->GetTransform().GetPosition();
		Vector3 dirNorm = dir.Normalised();
		float ang = Vector3::Dot(dirNorm, GuardForwardVector());
		float minAng = 0;
		minAng = AngleValue(minAng);
		if (ang > minAng) {
			float distance = dir.LengthSquared();
			if (distance < minDist) {
				minDist = distance;
				playerToChase = player;
			}
		}
	}

	return playerToChase;
}

int GuardObject::AngleValue(float minAng) {
	if (LevelManager::GetLevelManager()->GetSuspicionSystem()->GetGlobalSuspicionMetre()->GetGlobalSusMeter() > 50 && mCanSeePlayer == true) {
		minAng = 1.5;
	}
	else {
		minAng = 2;
	}
	return minAng;
}

bool GuardObject::CheckPolyDistance() {
	if (mDist < MIN_DIST_TO_NEXT_POS) {
		return true;
	}
	else {
		return false;
	}
}

void GuardObject::MoveTowardFocalPoint(float* endPos) {
	if (CheckPolyDistance() == true) {
		mNextPoly = QueryNavmesh(endPos);
	}
	else {
		delete[] endPos;
	}
	Vector3 dir = Vector3(mNextPoly[0], mNextPoly[1], mNextPoly[2]) - this->GetTransform().GetPosition();
	Vector3 dirNorm = dir.Normalised();
	mDist = dir.Length();
	LookTowardFocalPoint(dir);
	this->GetPhysicsObject()->AddForce(Vector3(dirNorm.x, 0, dirNorm.z) * mGuardSpeedMultiplier);
}

void GuardObject::LookTowardFocalPoint(Vector3 direction) {
	float angleOfPlayer = AngleFromFocalPoint(direction);

	if (angleOfPlayer < 0) {
		this->GetPhysicsObject()->AddTorque(Vector3(0, 20, 0));
		if (angleOfPlayer > -0.15) {
			this->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
		}
	}
	else if (angleOfPlayer > 0) {
		this->GetPhysicsObject()->AddTorque(Vector3(0, -20, 0));
		if (angleOfPlayer < 0.15) {
			this->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
		}
	}
}

void GuardObject::RunAfterPlayer(Vector3 dir) {
	LookTowardFocalPoint(dir);
	Vector3 dirNorm = dir.Normalised();
	this->GetPhysicsObject()->AddForce(Vector3(dirNorm.x, 0, dirNorm.z) * mGuardSpeedMultiplier);
}

void GuardObject::GrabPlayer() {
	mPlayer->GetPhysicsObject()->ClearForces();
}

float* GuardObject::QueryNavmesh(float* endPos) {
	float* startPos = new float[3] {this->GetTransform().GetPosition().x, this->GetTransform().GetPosition().y, this->GetTransform().GetPosition().z};
	float* halfExt = new float[3] {3, 5, 3};
	dtQueryFilter* filter = new dtQueryFilter();
	dtPolyRef* startRef = new dtPolyRef();
	dtPolyRef* endRef = new dtPolyRef();
	float* nearestPoint1 = new float[3];
	LevelManager::GetLevelManager()->GetBuilder()->GetNavMeshQuery()->findNearestPoly(startPos, halfExt, filter, startRef, nearestPoint1);
	LevelManager::GetLevelManager()->GetBuilder()->GetNavMeshQuery()->findNearestPoly(endPos, halfExt, filter, endRef, nearestPoint1);
	int* pathCount = new int;
	dtPolyRef* path = new dtPolyRef[1000];
	LevelManager::GetLevelManager()->GetBuilder()->GetNavMeshQuery()->findPath(*startRef, *endRef, startPos, endPos, filter, path, pathCount, 1000);
	float* firstPos = new float[3] {this->GetTransform().GetPosition().x, this->GetTransform().GetPosition().y, this->GetTransform().GetPosition().z};

	if (mDebugMode == true) {
		for (int i = 0; i < *pathCount; i++) {
			bool* isPosOverPoly = new bool;
			float* closestPos = new float[3];
			LevelManager::GetLevelManager()->GetBuilder()->GetNavMeshQuery()->closestPointOnPoly(path[i], firstPos, closestPos, isPosOverPoly);
			Debug::DrawLine(Vector3(firstPos[0], firstPos[1], firstPos[2]), Vector3(closestPos[0], closestPos[1], closestPos[2]));
			firstPos[0] = closestPos[0];
			firstPos[1] = closestPos[1];
			firstPos[2] = closestPos[2];
			delete isPosOverPoly;
			delete[] closestPos;
		}
	}

	delete[] startPos;
	delete[] halfExt;
	delete filter;
	delete startRef;
	delete endRef;
	delete[] nearestPoint1;

	bool* isPosOverPoly = new bool;
	float* closestPos = new float[3];
	LevelManager::GetLevelManager()->GetBuilder()->GetNavMeshQuery()->closestPointOnPoly(path[1], firstPos, closestPos, isPosOverPoly);
	delete isPosOverPoly;
	delete[] firstPos;
	delete[] path;
	if (*pathCount <= 1) {
		delete pathCount;
		return endPos;
	}
	delete pathCount;
	delete[] endPos;
	return closestPos;
}

void GuardObject::ApplyBuffToGuard(PlayerBuffs::buff buffToApply) {
	float buffDuration = LevelManager::GetLevelManager()->GetInventoryBuffSystem()->GetPlayerBuffsPtr()->GetBuffDuration(buffToApply);

	switch (buffToApply) {
	case PlayerBuffs::stun:
		mIsStunned = true;
		//TODO(erendgrmnc): if we want to add duration when the guard already has the status, handle it here.
#ifdef USEGL
		if (!mAppliedBuffs.contains(buffToApply)) {
			mAppliedBuffs.insert({ PlayerBuffs::buff::stun,  buffDuration });
		}
#endif
		break;
	default:;
	}
}

void GuardObject::RemoveBuffFromGuard(PlayerBuffs::buff removedBuff) {
	switch (removedBuff) {
	case PlayerBuffs::stun:
		mIsStunned = false;
		break;
	default:
		break;
	}
}

void GuardObject::CheckForDoors(float dt) {
	RayCollision closestCollision;
	Ray r = Ray(this->GetTransform().GetPosition(), GuardForwardVector());
	if (LevelManager::GetLevelManager()->GetGameWorld()->Raycast(r, closestCollision, true, this, true)) {
		mSightedDoor = (GameObject*)closestCollision.node;
		float dist = (mSightedDoor->GetTransform().GetPosition() - this->GetTransform().GetPosition()).LengthSquared();
		if (mSightedDoor->GetName() == "InteractableDoor" && dist < MIN_DIST_TO_NEXT_POS) {
			this->GetPhysicsObject()->ClearForces();
			if (mFumbleKeysCurrentTime <= 0) {
				mFumbleKeysCurrentTime = FUMBLE_KEYS_TIME;
				OpenDoor();
			}
			else {
				mFumbleKeysCurrentTime -= dt;
			}
		}
	}
}

void GuardObject::OpenDoor() {
	InteractableDoor* interactablePtr = (InteractableDoor*)mSightedDoor;
	if (interactablePtr != nullptr && interactablePtr->CanBeInteractedWith(NCL::CSC8503::InteractType::Use)) {
		interactablePtr->Interact(NCL::CSC8503::InteractType::Use, mSightedDoor);
	}
}

bool GuardObject::IsHighEnoughLocationSus() {
	mSmallestDistance = MAX_DIST_TO_SUS_LOCATION;
	mSmallestDistanceVector = Vector3(100000000, 100000000, 100000000);
	if (LevelManager::GetLevelManager()->GetSuspicionSystem()->GetLocationBasedSuspicion()->GetVec3LocationSusAmountMapPtr()->empty() == true) { return false; }

	for (auto it = LevelManager::GetLevelManager()->GetSuspicionSystem()->GetLocationBasedSuspicion()->GetVec3LocationSusAmountMapPtr()->begin();
		it != LevelManager::GetLevelManager()->GetSuspicionSystem()->GetLocationBasedSuspicion()->GetVec3LocationSusAmountMapPtr()->end(); it++) {
		if ((*it).second >= HIGH_SUSPICION) {
			float distance = ((*it).first - this->GetTransform().GetPosition()).LengthSquared();
			if (distance < mSmallestDistance) {
				mSmallestDistance = distance;
				mSmallestDistanceVector = (*it).first;
			}
		}
	}

	if (mSmallestDistance < MAX_DIST_TO_SUS_LOCATION) { return true; }
	else { return false; }
}

void GuardObject::SendAnnouncementToPlayer(){
#ifdef USEGL
	NetworkPlayer* networkPlayer = static_cast<NetworkPlayer*> (mPlayer);
	if (typeid(networkPlayer) == typeid(NetworkPlayer*))
		networkPlayer->AddAnnouncement(PlayerObject::CaughtByGuardAnnouncement, 5, networkPlayer->GetPlayerID());
	else
		mPlayer->AddAnnouncement(PlayerObject::CaughtByGuardAnnouncement, 5, mPlayer->GetPlayerID());
#endif
}

bool GuardObject::IsPlayerSprintingNearby() {
	mNearestSprintingPlayerDir = nullptr;
	for (PlayerObject* player : mPlayerList) {
		Vector3 playerDir = player->GetTransform().GetPosition() - this->GetTransform().GetPosition();
		float playerDist = playerDir.LengthSquared();
		if (player->GetGameOjbectState() == Sprint && playerDist <= MAX_DIST_TO_SUS_LOCATION) {
			if (mNearestSprintingPlayerDir == nullptr) { mNearestSprintingPlayerDir = new Vector3(playerDir); }
			else if (playerDist < (*mNearestSprintingPlayerDir).LengthSquared()){ *mNearestSprintingPlayerDir = playerDir; }
		}
	}
	if (mNearestSprintingPlayerDir != nullptr) { return true; }
	return false;
}

void GuardObject::BehaviourTree() {
	BehaviourSelector* FirstSelect = new BehaviourSelector("First Selector");
	BehaviourSequence* SeenPlayerSequence = new BehaviourSequence("Seen Player Sequence");
	BehaviourSelector* ChasePlayerSelector = new BehaviourSelector("Chase Player Selector");
	BehaviourSequence* CaughtPlayerSequence = new BehaviourSequence("Caught Player Sequence");
	mRootSequence->AddChild(FirstSelect);
	FirstSelect->AddChild(Patrol());
	FirstSelect->AddChild(CheckSusLocation());
	FirstSelect->AddChild(SeenPlayerSequence);
	SeenPlayerSequence->AddChild(ChasePlayerSelector);
	ChasePlayerSelector->AddChild(PointAtPlayer());
	ChasePlayerSelector->AddChild(ChasePlayerSetup());
	ChasePlayerSelector->AddChild(GoToLastKnownLocation());
	SeenPlayerSequence->AddChild(CaughtPlayerSequence);
	CaughtPlayerSequence->AddChild(ConfiscateItems());
	CaughtPlayerSequence->AddChild(SendToPrison());
}

void GuardObject::ExecuteBT() {
	BehaviourState state = mRootSequence->Execute(1.0f);
	if (state == Success || state == Failure) {
		mRootSequence->Reset();
		mHasCaughtPlayer = false;
	}
}

BehaviourAction* GuardObject::Patrol() {
	BehaviourAction* Patrol = new BehaviourAction("Patrol", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			state = Ongoing;
			mNextNode = mCurrentNode + 1;
			SetObjectState(Walk);
			if (mCurrentNode == mNodes.size() - 1) {
				mNextNode = 0;
			}
			else {
				mNextNode = mCurrentNode + 1;
			}

		}
		else if (state == Ongoing) {
			if (IsHighEnoughLocationSus() == true) { return Failure; }
			else if (IsPlayerSprintingNearby()) { 
				
				LookTowardFocalPoint(*mNearestSprintingPlayerDir); 
			}
			else if (mCanSeePlayer == false) {
				Vector3 direction = mNodes[mNextNode] - this->GetTransform().GetPosition();
				float* endPos = new float[3] { mNodes[mNextNode].x, mNodes[mNextNode].y, mNodes[mNextNode].z };
				MoveTowardFocalPoint(endPos);
				float dist = direction.LengthSquared();
				float changeInDist = mLastDist - dist;
				if (changeInDist < 0) { changeInDist *= -1; }
				if (changeInDist <= 0.2) { mDistCounter += 1; }
				if (dist < MIN_DIST_TO_NEXT_POS) {
					mCurrentNode = mNextNode;
					if (mCurrentNode == mNodes.size() - 1) { 
						mNextNode = 0; 
					}
					else { 
						mNextNode = mCurrentNode + 1; 
					}
				}else if (mDistCounter >= MAX_NUMBER_OF_FRAMES_GUARD_STUCK){
					mDistCounter = 0;
					mCurrentNode = mNextNode;
					if (mCurrentNode == mNodes.size() - 1) {
						mNextNode = 0;
					}
					else {
						mNextNode = mCurrentNode + 1;
					}
				}
				mLastDist = dist;
			}
			else if (mCanSeePlayer == true) { return Failure; }
		}
		
		return state;
		}
	);
	return Patrol;
}

BehaviourAction* GuardObject::CheckSusLocation() {
	BehaviourAction* CheckSusLocation = new BehaviourAction("Check Suspicious Location", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			state = Ongoing;
			SetObjectState(Walk);
		}
		else if (state == Ongoing) {
			if (mCanSeePlayer == true) {
				mSmallestDistance = MAX_DIST_TO_SUS_LOCATION;
				return Failure;
			}
			else {
				float* endPos = new float[3] {mSmallestDistanceVector.x, mSmallestDistanceVector.y, mSmallestDistanceVector.z};
				MoveTowardFocalPoint(endPos);
				if ((mSmallestDistanceVector - this->GetTransform().GetPosition()).LengthSquared() < MIN_DIST_TO_NEXT_POS) {
					LevelManager::GetLevelManager()->GetSuspicionSystem()->GetLocationBasedSuspicion()->RemoveSusLocation(mSmallestDistanceVector);
					mSmallestDistance = MAX_DIST_TO_SUS_LOCATION;
					return Success;
				}
			}
		}
		return state;
		}
	);
	return CheckSusLocation;
}

BehaviourAction* GuardObject::PointAtPlayer() {
	BehaviourAction* PointAtPlayer = new BehaviourAction("Point at Player", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			state = Ongoing;
			SetObjectState(Point);
		}
		else if (state == Ongoing) {
			if (mCanSeePlayer == true && mHasCaughtPlayer == false) {
				mPointTimer -= dt;
				Vector3 direction = mPlayer->GetTransform().GetPosition() - this->GetTransform().GetPosition();
				LookTowardFocalPoint(direction);
				this->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
				if (mPointTimer <= 0) {
#ifdef USEGL
					if (!SceneManager::GetSceneManager()->IsInSingleplayer()) {
						DebugNetworkedGame* game = reinterpret_cast<DebugNetworkedGame*>(SceneManager::GetSceneManager()->GetCurrentScene());
						if (mPlayer) {
							game->SendGuardSpotSoundPacket(mPlayer->GetPlayerID());
						}
					}
					else {
						if (mPlayer) {
							mPlayer->GetSoundObject()->TriggerSoundEvent();
						}
					}
#endif
					mPointTimer = POINTING_TIMER;
					return Failure;
				}
			}
			else if (mCanSeePlayer == false) { return Failure; }
		}
		return state;
		}
	);
	return PointAtPlayer;
}

BehaviourAction* GuardObject::ChasePlayerSetup() {
	BehaviourAction* ChasePlayer = new BehaviourAction("Chase Player", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			state = Ongoing;
			SetObjectState(Sprint);
		}
		else if (state == Ongoing) {
			if (mCanSeePlayer == true && mHasCaughtPlayer == false) {
				Vector3 direction = mPlayer->GetTransform().GetPosition() - this->GetTransform().GetPosition();
				float dist = direction.LengthSquared();

				if (dist < GUARD_CATCHING_DISTANCE_SQUARED) {
					LookTowardFocalPoint(direction);
					this->GetPhysicsObject()->AddForce(Vector3(direction.x, 0, direction.z));
					mHasCaughtPlayer = true;
					GrabPlayer();
					return Success;
				}
				else { 
					RunAfterPlayer(direction); 
				}
			}
			else {
				mLastKnownPos[0] = mPlayer->GetTransform().GetPosition().x;
				mLastKnownPos[1] = 0;
				mLastKnownPos[2] = mPlayer->GetTransform().GetPosition().z;
				return Failure;
			}
		}
		return state;
		}
	);
	return ChasePlayer;
}

BehaviourAction* GuardObject::GoToLastKnownLocation() {
	BehaviourAction* GoToLastKnownLocation = new BehaviourAction("Go To Last Known Location", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			state = Ongoing;
		}
		else if (state == Ongoing) {
			float* endPos = new float[3] {mLastKnownPos[0], mLastKnownPos[1], mLastKnownPos[2]};
			MoveTowardFocalPoint(endPos);
			Vector3 direction = Vector3(mLastKnownPos[0], mLastKnownPos[1], mLastKnownPos[2]) - this->GetTransform().GetPosition();
			float dist = direction.LengthSquared();
			if (mCanSeePlayer == true) {
				return Failure;
			}
			else if (dist < MIN_DIST_TO_NEXT_POS) {
				if (mCanSeePlayer == false) {
					return Success;
				}
			}
		}
		return state;
		}
	);
	return GoToLastKnownLocation;
}

BehaviourAction* GuardObject::ConfiscateItems() {
	BehaviourAction* ConfiscateItems = new BehaviourAction("Confiscate Items", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			mConfiscateItemsTime = 40;
			state = Ongoing;
		}
		else if (state == Ongoing) {
			if (mHasCaughtPlayer == true && mPlayerHasItems == true) {
				mConfiscateItemsTime -= dt;
				GrabPlayer();
				if (mConfiscateItemsTime == 0) {
					mPlayerHasItems = false;
					LevelManager::GetLevelManager()->GetInventoryBuffSystem()->GetPlayerInventoryPtr()->DropAllItemsFromPlayer(mPlayer->GetPlayerID());
					return Success;
				}
			}
			else if (mHasCaughtPlayer == true && mPlayerHasItems == false) {
				mConfiscateItemsTime -= dt;
				GrabPlayer();
				if (mConfiscateItemsTime == 0) {
					mPlayerHasItems = false;
					return Success;
				}
			}
			else {
				return Failure;
			}
		}
		return state;
		}
	);
	return ConfiscateItems;
}

BehaviourAction* GuardObject::SendToPrison() {
	BehaviourAction* SendToPrison = new BehaviourAction("Send to Prison", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			if (mCanSeePlayer == true && mHasCaughtPlayer == true && mPlayerHasItems == false) {
				mPlayer->GetTransform().SetPosition(LevelManager::GetLevelManager()->GetActiveLevel()->GetPrisonPosition());
				mPlayer->GetPhysicsObject()->ClearForces();
				SendAnnouncementToPlayer();
				LevelManager::GetLevelManager()->GetPrisonDoor()->SetIsOpen(false);
				mHasCaughtPlayer = false;
				return Success;
			}
			else {
				mHasCaughtPlayer = false;
				return Failure;
			}
			state = Ongoing;
		}
		else if (state == Ongoing) {

		}
		}
	);
	return SendToPrison;
}