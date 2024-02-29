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

using namespace NCL;
using namespace CSC8503;

GuardObject::GuardObject(const std::string& objectName) {
	mName = objectName;
	mRootSequence = new BehaviourSequence("Root Sequence");
	mCanSeePlayer = false;
	mHasCaughtPlayer = false;
	mPlayerHasItems = true;
	mIsStunned = false;
	BehaviourTree();
	mDist = 0;
	mNextPoly = 0;
}

GuardObject::~GuardObject() {
	delete mRootSequence;
	delete[] mNextPoly;
}

void GuardObject::UpdateObject(float dt) {
	if (!mIsStunned) {
		RaycastToPlayer();
		ExecuteBT();
	}
	else {
		Debug::Print("Guard Is Stunned! ", Vector2(10, 40));
	}
	HandleAppliedBuffs(dt);
}

void GuardObject::RaycastToPlayer() {
	mPlayer = dynamic_cast<PlayerObject*>(mPlayer);
	Vector3 dir = (mPlayer->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Normalised();
	float ang = Vector3::Dot(dir, GuardForwardVector());
	if (ang > 2) {
		RayCollision closestCollision;
		Ray r = Ray(this->GetTransform().GetPosition(), dir);
		if (LevelManager::GetLevelManager()->GetGameWorld()->Raycast(r, closestCollision, true, this)) {
			mSightedObject = (GameObject*)closestCollision.node;
			Debug::DrawLine(this->GetTransform().GetPosition(), closestCollision.collidedAt);
			if (mSightedObject == mPlayer) {
				mCanSeePlayer = true;
			}
			else {
				mCanSeePlayer = false;
			}
		}
		else {
			mCanSeePlayer = false;
		}
	}
	else {
		mCanSeePlayer = false;
		mSightedObject = nullptr;
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
		this->GetPhysicsObject()->AddTorque(Vector3(0,10, 0));
		if (angleOfPlayer > -0.1) {
			this->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
		}
	}
	else if (angleOfPlayer > 0) {
		this->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		if (angleOfPlayer < 0.1) {
			this->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
		}
	}
}

void GuardObject::GrabPlayer() {
	Vector3 direction = this->GetTransform().GetPosition() - mPlayer->GetTransform().GetPosition();
	mPlayer->GetPhysicsObject()->AddForce(Vector3(direction.x, 0, direction.z));
}

float* GuardObject::QueryNavmesh(float* endPos) {
	float* startPos = new float[3] {this->GetTransform().GetPosition().x, this->GetTransform().GetPosition().y, this->GetTransform().GetPosition().z};
	float* halfExt = new float[3] {3, 5, 3};
	dtQueryFilter* filter = new dtQueryFilter();
	dtPolyRef* startRef = new dtPolyRef();
	dtPolyRef* endRef = new dtPolyRef();
	float* nearestPoint1 = new float[3];
	LevelManager::GetLevelManager()->GetBuilder()->GetNavMeshQuery()->findNearestPoly(startPos, halfExt, filter, startRef, nearestPoint1);
	//float* nearestPoint2 = new float[3];
	LevelManager::GetLevelManager()->GetBuilder()->GetNavMeshQuery()->findNearestPoly(endPos, halfExt, filter, endRef, nearestPoint1);
	int* pathCount = new int;
	dtPolyRef* path = new dtPolyRef[1000];
	LevelManager::GetLevelManager()->GetBuilder()->GetNavMeshQuery()->findPath(*startRef, *endRef, startPos, endPos, filter, path, pathCount, 1000);
	float* firstPos = new float[3] {this->GetTransform().GetPosition().x, this->GetTransform().GetPosition().y, this->GetTransform().GetPosition().z};
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
		if (!mAppliedBuffs.contains(buffToApply)) {
			mAppliedBuffs.insert({ PlayerBuffs::buff::stun,  buffDuration });
		}
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

void GuardObject::BehaviourTree() {
	BehaviourSelector* FirstSelect = new BehaviourSelector("First Selector");
	BehaviourSequence* CaughtPlayerSequence = new BehaviourSequence("Caught Player Sequence");
	mRootSequence->AddChild(FirstSelect);
	FirstSelect->AddChild(Patrol());
	FirstSelect->AddChild(ChasePlayerSetup());
	FirstSelect->AddChild(CaughtPlayerSequence);
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
			mObjectState = Walk;
			if (mCurrentNode == mNodes.size() - 1) {
				mNextNode = 0;
			}
			else {
				mNextNode = mCurrentNode + 1;
			}

		}
		else if (state == Ongoing) {
			if (mCanSeePlayer == false) {

				mGuardSpeedMultiplier = 25;
				Vector3 direction = mNodes[mNextNode] - this->GetTransform().GetPosition();
				float* endPos = new float[3] { mNodes[mNextNode].x, mNodes[mNextNode].y, mNodes[mNextNode].z };
				MoveTowardFocalPoint(endPos);
				float dist = direction.LengthSquared();
				if (dist < 36) {
					mCurrentNode = mNextNode;
					if (mCurrentNode == mNodes.size() - 1) {
						mNextNode = 0;
					}
					else {
						mNextNode = mCurrentNode + 1;
					}
				}
			}
			else if (mCanSeePlayer == true) {
				return Failure;

			}
		}
		return state;
		}
	);
	return Patrol;
}

BehaviourAction* GuardObject::ChasePlayerSetup() {
	BehaviourAction* ChasePlayer = new BehaviourAction("Chase Player", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			state = Ongoing;
			mObjectState = Sprint;
		}
		else if (state == Ongoing) {
			if (mCanSeePlayer == true && mHasCaughtPlayer == false) {

				int GuardCatchingDistanceSquared = 36;
				mGuardSpeedMultiplier = 40;
				Vector3 direction = mPlayer->GetTransform().GetPosition() - this->GetTransform().GetPosition();

				float dist = direction.LengthSquared();
				if (dist < GuardCatchingDistanceSquared) {
					LookTowardFocalPoint(direction);
					this->GetPhysicsObject()->AddForce(Vector3(direction.x, 0, direction.z));
					mHasCaughtPlayer = true;
					return Failure;
				}
				else {
					float* endPos = new float[3] { mPlayer->GetTransform().GetPosition().x, mPlayer->GetTransform().GetPosition().y, mPlayer->GetTransform().GetPosition().z };
					MoveTowardFocalPoint(endPos);
				}
			}
			else if (mCanSeePlayer == false && mHasCaughtPlayer == false) {
				return Success;
			}
			else {
				return Failure;
			}
		}
		return state;
		}
	);
	return ChasePlayer;
}

BehaviourAction* GuardObject::ConfiscateItems() {
	BehaviourAction* ConfiscateItems = new BehaviourAction("Confiscate Items", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			mConfiscateItemsTime = 60;
			state = Ongoing;
		}
		else if (state == Ongoing) {
			if (mCanSeePlayer == true && mHasCaughtPlayer == true && mPlayerHasItems == true) {
				mConfiscateItemsTime -= dt;
				GrabPlayer();
				if (mConfiscateItemsTime == 0) {
					mPlayerHasItems = false;
					return Success;
				}
			}
			else if (mCanSeePlayer == true && mHasCaughtPlayer == true && mPlayerHasItems == false) {
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
				mPlayer->GetPhysicsObject()->SetLinearVelocity(Vector3(0,0,0));
				mPlayer->ClosePrisonDoor();
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