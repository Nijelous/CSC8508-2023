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
	mPlayerHasItems = true;
	BehaviourTree();
}

GuardObject::~GuardObject() {
	delete mRootSequence;
}

void GuardObject::UpdateObject(float dt) {
	RaycastToPlayer();
	ExecuteBT();
}

void GuardObject::RaycastToPlayer() {
	mPlayer = dynamic_cast<PlayerObject*>(mPlayer);
	Vector3 dir = (mPlayer->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Normalised();
	float ang = Vector3::Dot(dir, GuardForwardVector());
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

void GuardObject::MoveTowardFocalPoint(Vector3 direction) {
	Vector3 dirNorm = direction.Normalised();
	this->GetPhysicsObject()->AddForce(Vector3(dirNorm.x, 0, dirNorm.z) * mGuardSpeedMultiplier);

}

void GuardObject::LookTowardFocalPoint(Vector3 direction) {
	float angleOfPlayer = AngleFromFocalPoint(direction);

	if (angleOfPlayer < 0) {
		this->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
	}
	else if (angleOfPlayer > 0) {
		this->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
	}
}

void GuardObject::GrabPlayer() {
	Vector3 direction = this->GetTransform().GetPosition() - mPlayer->GetTransform().GetPosition();
	mPlayer->GetPhysicsObject()->AddForce(Vector3(direction.x, 0, direction.z));
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
				LookTowardFocalPoint(direction);
				MoveTowardFocalPoint(direction);
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
				
				int GuardCatchingDistanceSquared = 25;
				mGuardSpeedMultiplier = 40;
				Vector3 direction = mPlayer->GetTransform().GetPosition() - this->GetTransform().GetPosition();

				LookTowardFocalPoint(direction);

				float dist = direction.LengthSquared();
				if (dist < GuardCatchingDistanceSquared) {
					this->GetPhysicsObject()->AddForce(Vector3(direction.x, 0, direction.z));
					mHasCaughtPlayer = true;
					return Failure;
				}
				else {
					MoveTowardFocalPoint(direction);
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
				mPlayer->GetTransform().SetPosition(mPrisonPosition);
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