#include "GuardObject.h"
#include "PlayerObject.h"
#include "Ray.h"
#include "Debug.h"
#include "PhysicsObject.h"
#include "BehaviourSelector.h"

using namespace NCL;
using namespace CSC8503;

GuardObject::GuardObject(const std::string& objectName) {
	mName = objectName;
	mRootSequence = new BehaviourSequence("Root Sequence");
	mCanSeePlayer = false;
	mHasCaughtPlayer = false;
	mHasItems = true;
	BehaviourTree();
}

GuardObject::~GuardObject() {
	delete mPlayer;
	delete mWorld;
	delete mSightedObject;
	delete mRootSequence;
}

void GuardObject::UpdateObject(float dt) {
	RaycastToPlayer();
	ExecuteBT();
}

void GuardObject::RaycastToPlayer() {
	mPlayer = dynamic_cast<PlayerObject*>(mPlayer);
	Vector3 dir = (mPlayer->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Normalised();
	float ang = Vector3::Dot(dir, AngleOfSight());
	if (ang > 2) {
		RayCollision closestCollision;
		Ray r = Ray(this->GetTransform().GetPosition(), dir);
		if (mWorld->Raycast(r, closestCollision, true, this)) {
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

Vector3 GuardObject::AngleOfSight() {
	Vector3 rightAxis = this->mTransform.GetMatrix().GetColumn(0);
	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	return fwdAxis;
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
		}
		else if (state == Ongoing) {
			if (mCanSeePlayer == false) {
				return Success;
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
		}
		else if (state == Ongoing){
			if (mCanSeePlayer == true && mHasCaughtPlayer == false) {
				int GuardCatchingDistanceSquared = 25;
				Vector3 direction = mPlayer->GetTransform().GetPosition() - this->GetTransform().GetPosition();
				this->GetPhysicsObject()->AddForce(Vector3(direction.x, 0, direction.z));
				float dist = direction.LengthSquared();
				if (dist < GuardCatchingDistanceSquared) {
					mHasCaughtPlayer = true;
					return Failure;
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
			mTimer = 60;
			state = Ongoing;
		}
		else if (state == Ongoing) {
			if (mCanSeePlayer == true && mHasCaughtPlayer == true && mHasItems == true) {
				mTimer -= dt;
				Vector3 direction = mPlayer->GetTransform().GetPosition();
				//mPlayer->GetPhysicsObject()->AddForce(Vector3(direction.x, 0, direction.z));
				mPlayer->GetTransform().SetPosition(Vector3(direction.x, direction.y, direction.z));
				if (mTimer == 0) {
					mHasItems = false;
					return Success;
				}
			}
			else if (mCanSeePlayer == true && mHasCaughtPlayer == true && mHasItems == false) {
				mTimer -= dt;
				Vector3 direction = mPlayer->GetTransform().GetPosition();
				//mPlayer->GetPhysicsObject()->AddForce(Vector3(direction.x, 0, direction.z));
				mPlayer->GetTransform().SetPosition(Vector3(direction.x, direction.y, direction.z));
				if (mTimer == 0) {
					mHasItems = false;
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
			if (mCanSeePlayer == true && mHasCaughtPlayer == true && mHasItems == false) {
				Vector3 prisonLocation = Vector3(mPlayer->GetTransform().GetPosition().x, 100, mPlayer->GetTransform().GetPosition().z);
				mPlayer->GetTransform().SetPosition(prisonLocation);
				return Success;
			}
			else {
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