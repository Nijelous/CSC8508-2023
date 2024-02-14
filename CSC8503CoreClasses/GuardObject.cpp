#include "GuardObject.h"
#include "../CSC8503/PlayerObject.h"
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

Quaternion GuardObject::VectorToQuaternion(Vector3 directionVector) {
	directionVector.Normalise();
	directionVector.y = 0;
	Vector3 xAxis = Vector3::Cross(directionVector, Vector3(0, 1, 0));
	xAxis.Normalise();
	Vector3 yAxis = Vector3::Cross(directionVector, xAxis);
	yAxis.Normalise();

	Matrix3 m = Matrix3();
	Vector3 column1 = Vector3(xAxis.x, yAxis.x, directionVector.x);
	Vector3 column2 = Vector3(xAxis.y, yAxis.y, directionVector.y);
	Vector3 column3 = Vector3(xAxis.z, yAxis.z, directionVector.z);
	m.SetColumn(0, column1);
	m.SetColumn(1, column2);
	m.SetColumn(2, column3);

	return Matrix3ToQuaternion(m);
}

Quaternion GuardObject::Matrix3ToQuaternion(Matrix3 m) {
	Quaternion rotation = Quaternion();
	float trace = m.GetColumn(0)[0] + m.GetColumn(1)[1] + m.GetColumn(2)[2];
	if (trace > 0) {
		float s = 0.5f / sqrtf(trace + 1.0f);
		rotation.w = 0.25f / s;
		rotation.x = (m.GetColumn(2)[1] - m.GetColumn(1)[2]) * s;
		rotation.y = (m.GetColumn(0)[2] - m.GetColumn(2)[0]) * s;
		rotation.z = (m.GetColumn(1)[0] - m.GetColumn(0)[1]) * s;
	}
	else {
		if (m.GetColumn(0)[0] > m.GetColumn(1)[1] && m.GetColumn(0)[0] > m.GetColumn(2)[2]) {
			float s = 2.0f * sqrtf(1.0f + m.GetColumn(0)[0] - m.GetColumn(1)[1] - m.GetColumn(2)[2]);
			rotation.w = (m.GetColumn(2)[1] - m.GetColumn(1)[2]) / s;
			rotation.x = 0.25f * s;
			rotation.y = (m.GetColumn(0)[1] + m.GetColumn(1)[0]) / s;
			rotation.z = (m.GetColumn(0)[2] + m.GetColumn(2)[0]) / s;
		}
		else if (m.GetColumn(1)[1] > m.GetColumn(2)[2]) {
			float s = 2.0f * sqrtf(1.0f + m.GetColumn(1)[1] - m.GetColumn(0)[0] - m.GetColumn(2)[2]);
			rotation.w = (m.GetColumn(0)[2] - m.GetColumn(2)[0]) / s;
			rotation.x = (m.GetColumn(0)[1] + m.GetColumn(1)[0]) / s;
			rotation.y = 0.25f * s;
			rotation.z = (m.GetColumn(1)[2] + m.GetColumn(2)[1]) / s;
		}
		else {
			float s = 2.0f * sqrtf(1.0f + m.GetColumn(2)[2] - m.GetColumn(0)[0] - m.GetColumn(1)[1]);
			rotation.w = (m.GetColumn(1)[0] - m.GetColumn(0)[1]) / s;
			rotation.x = (m.GetColumn(0)[2] + m.GetColumn(2)[0]) / s;
			rotation.y = (m.GetColumn(1)[2] + m.GetColumn(2)[1]) / s;
			rotation.z = 0.25f * s;
		}
	}
	return rotation;
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
				//std::cout << "Lost em";
  
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
				int GuardSpeedMultiplyer = 30;
				Vector3 direction = mPlayer->GetTransform().GetPosition() - this->GetTransform().GetPosition();
				Vector3 dirNorm = direction.Normalised();
				this->GetTransform().SetOrientation(VectorToQuaternion(direction) * Quaternion(1.0f, 0.0f,0.0f,0.0f));

				this->GetPhysicsObject()->AddForce(Vector3(dirNorm.x, 0, dirNorm.z) * GuardSpeedMultiplyer);

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
			mConfiscateItemsTime = 60;
			state = Ongoing;
		}
		else if (state == Ongoing) {
			if (mCanSeePlayer == true && mHasCaughtPlayer == true && mPlayerHasItems == true) {
				mConfiscateItemsTime -= dt;
				std::cout << "Caught";
				Vector3 direction = this->GetTransform().GetPosition() - mPlayer->GetTransform().GetPosition();
				mPlayer->GetPhysicsObject()->AddForce(Vector3(direction.x, 0, direction.z));
				if (mConfiscateItemsTime == 0) {
					mPlayerHasItems = false;
					return Success;
				}
			}
			else if (mCanSeePlayer == true && mHasCaughtPlayer == true && mPlayerHasItems == false) {
				mConfiscateItemsTime -= dt;
				Vector3 direction = this->GetTransform().GetPosition() - mPlayer->GetTransform().GetPosition();
				mPlayer->GetPhysicsObject()->AddForce(Vector3(direction.x, 0, direction.z));
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
				Vector3 prisonLocation = Vector3(mPlayer->GetTransform().GetPosition().x, 100, mPlayer->GetTransform().GetPosition().z);
				mPlayer->GetTransform().SetPosition(prisonLocation);
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