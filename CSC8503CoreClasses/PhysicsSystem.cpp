#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "Quaternion.h"

#include "Constraint.h"
#include "CollisionDetection.h"
#include "Debug.h"
#include "Window.h"
#include <functional>
using namespace NCL;
using namespace CSC8503;

PhysicsSystem::PhysicsSystem(GameWorld& g) : mGameWorld(g) {
	mApplyGravity = false;
	mDTOffset = 0.0f;
	mGlobalDamping = 0.995f;
	SetGravity(Vector3(0.0f, -9.8f, 0.0f));
	mStaticTree = QuadTree<GameObject*>(Vector2(mBroadphaseX, mBroadphaseZ), 7, 6);
}

PhysicsSystem::~PhysicsSystem() {
}

void PhysicsSystem::SetGravity(const Vector3& g) {
	mGravity = g;
}

void PhysicsSystem::SetNewBroadphaseSize(const Vector3& levelSize) {
	mBroadphaseX = 64;
	mBroadphaseZ = 64;
	while(levelSize.x > mBroadphaseX){
		mBroadphaseX *= 2;
	}
	while (levelSize.z > mBroadphaseZ) {
		mBroadphaseZ *= 2;
	}
	mStaticTree = QuadTree<GameObject*>(Vector2(mBroadphaseX, mBroadphaseZ), 7, 6);
}

/*

If the 'game' is ever reset, the PhysicsSystem must be
'cleared' to remove any old collisions that might still
be hanging around in the collision list. If your engine
is expanded to allow objects to be removed from the world,
you'll need to iterate through this collisions list to remove
any collisions they are in.

*/
void PhysicsSystem::Clear() {
	mAllCollisions.clear();
	mDynamicObjectList.clear();
}

/*

This is the core of the physics engine update

*/

bool useSimpleContainer = false;

int constraintIterationCount = 10;

//This is the fixed timestep we'd LIKE to have
const int   idealHZ = 120;
const float idealDT = 1.0f / idealHZ;

/*
This is the fixed update we actually have...
If physics takes too long it starts to kill the framerate, it'll drop the
iteration count down until the FPS stabilises, even if that ends up
being at a low rate.
*/
int realHZ = idealHZ;
float realDT = idealDT;

void PhysicsSystem::Update(float dt) {
	mDTOffset += dt; //We accumulate time delta here - there might be remainders from previous frame!

	GameTimer t;
	t.GetTimeDeltaSeconds();

	if (mUseBroadPhase) {
		UpdateObjectAABBs();
	}
	int iteratorCount = 0;
	while (mDTOffset > realDT) {
		IntegrateAccel(realDT); //Update accelerations from external forces
		if (mUseBroadPhase) {
			BroadPhase();
			NarrowPhase();
		}
		else {
			BasicCollisionDetection();
		}

		//This is our simple iterative solver - 
		//we just run things multiple times, slowly moving things forward
		//and then rechecking that the constraints have been met		
		float constraintDt = realDT / (float)constraintIterationCount;
		for (int i = 0; i < constraintIterationCount; ++i) {
			UpdateConstraints(constraintDt);
		}
		IntegrateVelocity(realDT); //update positions from new velocity changes

		mDTOffset -= realDT;
		iteratorCount++;
	}

	ClearForces();	//Once we've finished with the forces, reset them to zero

	UpdateCollisionList(); //Remove any old collisions

	t.Tick();
	float updateTime = t.GetTimeDeltaSeconds();

	//Uh oh, physics is taking too long...
	if (updateTime > realDT) {
		realHZ /= 2;
		realDT *= 2;
		std::cout << "Dropping iteration count due to long physics time...(now " << realHZ << ")\n";
	}
	else if (dt * 2 < realDT) { //we have plenty of room to increase iteration count!
		int temp = realHZ;
		realHZ *= 2;
		realDT /= 2;

		if (realHZ > idealHZ) {
			realHZ = idealHZ;
			realDT = idealDT;
		}
		if (temp != realHZ) {
			std::cout << "Raising iteration count due to short physics time...(now " << realHZ << ")\n";
		}
	}
}

/*
Later on we're going to need to keep track of collisions
across multiple frames, so we store them in a set.

The first time they are added, we tell the objects they are colliding.
The frame they are to be removed, we tell them they're no longer colliding.

From this simple mechanism, we we build up gameplay interactions inside the
OnCollisionBegin / OnCollisionEnd functions (removing health when hit by a
rocket launcher, gaining a point when the player hits the gold coin, and so on).
*/
void PhysicsSystem::UpdateCollisionList() {
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = mAllCollisions.begin(); i != mAllCollisions.end(); ) {
		if ((*i).framesLeft == mNumCollisionFrames) {
			i->a->OnCollisionBegin(i->b);
			i->b->OnCollisionBegin(i->a);
		}

		CollisionDetection::CollisionInfo& in = const_cast<CollisionDetection::CollisionInfo&>(*i);
		in.framesLeft--;

		if ((*i).framesLeft < 0) {
			i->a->OnCollisionEnd(i->b);
			i->b->OnCollisionEnd(i->a);
			i = mAllCollisions.erase(i);
		}
		else {
			++i;
		}
	}
}

void PhysicsSystem::UpdateObjectAABBs() {
	mGameWorld.OperateOnContents(
		[](GameObject* g) {
			g->UpdateBroadphaseAABB();
		}
	);
}

/*

This is how we'll be doing collision detection in tutorial 4.
We step thorugh every pair of objects once (the inner for loop offset
ensures this), and determine whether they collide, and if so, add them
to the collision set for later processing. The set will guarantee that
a particular pair will only be added once, so objects colliding for
multiple frames won't flood the set with duplicates.
*/
void PhysicsSystem::BasicCollisionDetection() {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	mGameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; i++) {
		if ((*i)->GetPhysicsObject() == nullptr)
			continue;
		for (auto j = i + 1; j != last; j++) {
			if ((*j)->GetPhysicsObject() == nullptr)
				continue;
			CollisionDetection::CollisionInfo info;
			if (CollisionDetection::ObjectIntersection(*i, *j, info)) {
				if (!((*i)->GetBoundingVolume()->applyPhysics && (*j)->GetBoundingVolume()->applyPhysics))
					continue;
				float j = ImpulseResolveCollision(*info.a, *info.b, info.point);
				FrictionImpulse(*info.a, *info.b, info.point, j);
				info.framesLeft = mNumCollisionFrames;
				mAllCollisions.insert(info);
			}
		}
	}
}

/*

In tutorial 5, we start determining the correct response to a collision,
so that objects separate back out.

*/
float PhysicsSystem::ImpulseResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {
	PhysicsObject* physA = a.GetPhysicsObject();
	PhysicsObject* physB = b.GetPhysicsObject();

	float totalMass = physA->GetInverseMass() + physB->GetInverseMass();

	// both objects are static
	if (totalMass == 0)
		return 0;

	SeperateObjects(a, b, p, totalMass);

	// local collision points
	Vector3 relativeA = p.localA;
	Vector3 relativeB = p.localB;

	Vector3 contactVelocity = CalculateCollisionVelocity(relativeA, relativeB, physA, physB);

	float impulseForce = Vector3::Dot(contactVelocity, p.normal);
	float angularEffect = CalculateInertia(physA, physB, relativeA, relativeB, p.normal);

	float cRestitution = GetCollisionElasticity(*physA, *physB); // loss of kinetic energy

	float j = (-(1.0f + cRestitution) * impulseForce) / (totalMass + angularEffect);
	Vector3 fullImpulse = p.normal * j;

	// apply impulse in opposite directions for collision responce
	physA->ApplyLinearImpulse(-fullImpulse);
	physB->ApplyLinearImpulse(fullImpulse);
	physA->ApplyAngularImpulse(Vector3::Cross(relativeA, -fullImpulse));
	physB->ApplyAngularImpulse(Vector3::Cross(relativeB, fullImpulse));

	return j;
}

float PhysicsSystem::GetCollisionElasticity(PhysicsObject objectA, PhysicsObject objectB) const {
	return objectA.GetElasticity() * objectB.GetElasticity();
}

void PhysicsSystem::FrictionImpulse(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p, float j) const {
	PhysicsObject* physA = a.GetPhysicsObject();
	PhysicsObject* physB = b.GetPhysicsObject();
	float totalMass = physA->GetInverseMass() + physB->GetInverseMass();
	if (!CheckFrictionShuldBeApplied(physA->GetLinearVelocity().Length(), physB->GetLinearVelocity().Length(), totalMass))
		return;
	Vector3 relativeA = p.localA;
	Vector3 relativeB = p.localB;

	Vector3 contactVelocity = CalculateCollisionVelocity(relativeA, relativeB, physA, physB);

	Vector3 tangent = CalculateFrictionDirection(contactVelocity, p.normal);
	float angularEffect = CalculateInertia(physA, physB, relativeA, relativeB, tangent);

	float impulseForce = Vector3::Dot(contactVelocity, tangent);

	j = abs(j);

	float friction = CalculateFriction(physA, physB);

	Vector3 fullFrictionImpulse = CalculateFrictionImpulse(tangent, j, friction);

	// apply impulse opposite direction incoming force
	physA->ApplyLinearImpulse(fullFrictionImpulse);
	physB->ApplyLinearImpulse(-fullFrictionImpulse);
	if (!(GetIsCapsule(a)))
		physA->ApplyAngularImpulse(Vector3::Cross(relativeA, fullFrictionImpulse));
	if (!(GetIsCapsule(b)))
		physB->ApplyAngularImpulse(Vector3::Cross(relativeB, -fullFrictionImpulse));
}

bool PhysicsSystem::GetIsCapsule(GameObject& obj) const {
	if (obj.GetBoundingVolume()->type == VolumeType::Capsule)
		return true;
	return false;
}

void PhysicsSystem::SeperateObjects(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p, float totalMass) const {
	Transform& transformA = a.GetTransform();
	Transform& transformB = b.GetTransform();

	transformA.SetPosition(transformA.GetPosition() - (p.normal * p.penetration * (a.GetPhysicsObject()->GetInverseMass() / totalMass)));
	transformB.SetPosition(transformB.GetPosition() + (p.normal * p.penetration * (b.GetPhysicsObject()->GetInverseMass() / totalMass)));
}

bool PhysicsSystem::CheckFrictionShuldBeApplied(float aVelocity, float bVelocity, float totalMass) const {
	if (aVelocity < 1 && bVelocity < 1)
		return false;
	if (totalMass <= 0)
		return false;
	return true;
}

Vector3 PhysicsSystem::CalculateCollisionVelocity(Vector3 contactPointA, Vector3 contactPointB, PhysicsObject* physA, PhysicsObject* physB) const {
	Vector3 angVelocityA = Vector3::Cross(physA->GetAngularVelocity(), contactPointA);
	Vector3 angVelocityB = Vector3::Cross(physB->GetAngularVelocity(), contactPointB);

	Vector3 fullVelocityA = physA->GetLinearVelocity() + angVelocityA;
	Vector3 fullVelocityB = physB->GetLinearVelocity() + angVelocityB;

	return fullVelocityB - fullVelocityA;
}

Vector3 PhysicsSystem::CalculateFrictionDirection(Vector3 contactVelocity, Vector3 collisionNormal) const {
	// get direction of friction
	float impulseForce = Vector3::Dot(contactVelocity, collisionNormal);
	Vector3 tangent = contactVelocity - (collisionNormal * impulseForce);
	tangent.Normalise();
	return tangent;
}

float PhysicsSystem::CalculateInertia(PhysicsObject* physA, PhysicsObject* physB, Vector3 contactPointA, Vector3 contactPointB, Vector3 angle) const {
	Vector3 inertiaA = Vector3::Cross(physA->GetInertiaTensor() * Vector3::Cross(contactPointA, angle), contactPointA);
	Vector3 inertiaB = Vector3::Cross(physB->GetInertiaTensor() * Vector3::Cross(contactPointB, angle), contactPointB);
	return Vector3::Dot(inertiaA + inertiaB, angle);
}

float PhysicsSystem::CalculateFriction(PhysicsObject* physA, PhysicsObject* physB) const {
	float frictionA;
	float frictionB;

	if (physA->GetForce() == Vector3(0, 0, 0) && physA->GetInverseMass() > 0)
		frictionA = physA->GetStaticFriction();
	else
		frictionA = physA->GetDynamicFriction();

	if (physB->GetForce() == Vector3(0, 0, 0) && physB->GetInverseMass() > 0)
		frictionB = physB->GetStaticFriction();
	else
		frictionB = physB->GetDynamicFriction();
	return (frictionA + frictionB) / 2;
}

Vector3 PhysicsSystem::CalculateFrictionImpulse(Vector3 tangent, float j, float friction) const {
	Vector3 fullFrictionImpulse;

	fullFrictionImpulse = tangent * j * friction;

	return fullFrictionImpulse;
}

/*

Later, we replace the BasicCollisionDetection method with a broadphase
and a narrowphase collision detection method. In the broad phase, we
split the world up using an acceleration structure, so that we can only
compare the collisions that we absolutely need to.

*/
void PhysicsSystem::BroadPhase() {
	// clear last frames collisions
 	mBroadphaseCollisions.clear();

	// create quadtree to store all objects
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	mGameWorld.GetObjectIterators(first, last);
	if (first == last) return;
	if(mStaticTree.Empty()) {
		for (auto i = first; i != last; i++) {
			Vector3 halfSizes;
			if (!(*i)->GetBroadphaseAABB(halfSizes)) continue;
			if ((*i)->GetCollisionLayer() & STATIC_COLLISION_LAYERS) {
				Vector3 pos = (*i)->GetTransform().GetPosition() + (*i)->GetBoundingVolume()->GetOffset();
				mStaticTree.Insert(*i, pos, halfSizes, true);
			}
			else {
				mDynamicObjectList.push_back(*i);
			}
		}
	}
	for (int i = 0; i < mDynamicObjectList.size(); i++) {
		if (!mDynamicObjectList[i]->HasPhysics()) continue;
		Vector3 halfSize;
		mDynamicObjectList[i]->GetBroadphaseAABB(halfSize);
		mStaticTree.OperateOnLeaf([&](std::list<QuadTreeEntry<GameObject*>>& data) {
			CollisionDetection::CollisionInfo info;
			for (auto j = data.begin(); j != data.end(); j++) {
				if (!(*j).object->HasPhysics()) continue;
				info.a = std::min(mDynamicObjectList[i], (*j).object);
				info.b = std::max(mDynamicObjectList[i], (*j).object);
				Vector3 halfSizeA;
				Vector3 halfSizeB;
				info.a->GetBroadphaseAABB(halfSizeA);
				info.b->GetBroadphaseAABB(halfSizeB);
				halfSizeA.y = 1000.0f;
				halfSizeB.y = 1000.0f;
				if (!CollisionDetection::AABBTest(info.a->GetTransform().GetPosition() + info.a->GetBoundingVolume()->GetOffset(), 
					info.b->GetTransform().GetPosition() + info.b->GetBoundingVolume()->GetOffset(),
					halfSizeA, halfSizeB)) continue;
				if (mDynamicObjectList[i]->GetCollisionLayer() & Npc && (*j).object->GetCollisionLayer() & Collectable) {
					continue;
				}
				mBroadphaseCollisions.insert(info);
			}
			}, mDynamicObjectList[i]->GetTransform().GetPosition(), halfSize);
		for (int j = i; j < mDynamicObjectList.size(); j++) {
			if (!mDynamicObjectList[j]->HasPhysics()) continue;
			CollisionDetection::CollisionInfo info;
			info.a = std::min(mDynamicObjectList[i], mDynamicObjectList[j]);
			info.b = std::max(mDynamicObjectList[i], mDynamicObjectList[j]);
			Vector3 halfSizeA;
			Vector3 halfSizeB;
			info.a->GetBroadphaseAABB(halfSizeA);
			info.b->GetBroadphaseAABB(halfSizeB);
			halfSizeA.y = 1000.0f;
			halfSizeB.y = 1000.0f;
			if (!CollisionDetection::AABBTest(info.a->GetTransform().GetPosition() + info.a->GetBoundingVolume()->GetOffset(),
				info.b->GetTransform().GetPosition() + info.b->GetBoundingVolume()->GetOffset(),
				halfSizeA, halfSizeB)) continue;
			mBroadphaseCollisions.insert(info);
		}
	}
}


/*

The broadphase will now only give us likely collisions, so we can now go through them,
and work out if they are truly colliding, and if so, add them into the main collision list
*/
void PhysicsSystem::NarrowPhase() {
	// iteratr through all collisions added and if collision then call impulse resolve collision
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = mBroadphaseCollisions.begin(); i != mBroadphaseCollisions.end(); i++) {
		CollisionDetection::CollisionInfo info = *i;
		
		if (CollisionDetection::ObjectIntersection(info.a, info.b, info)) {
			info.framesLeft = mNumCollisionFrames;
			if (!(info.a->GetCollisionLayer() & NO_COLLISION_RESOLUTION || info.b->GetCollisionLayer() & NO_COLLISION_RESOLUTION)) {
				float j = ImpulseResolveCollision(*info.a, *info.b, info.point);
				FrictionImpulse(*info.a, *info.b, info.point, j);
			}
			mAllCollisions.insert(info);
		}
	}
}

/*
Integration of acceleration and velocity is split up, so that we can
move objects multiple times during the course of a PhysicsUpdate,
without worrying about repeated forces accumulating etc.

This function will update both linear and angular acceleration,
based on any forces that have been accumulated in the objects during
the course of the previous game frame.
*/
void PhysicsSystem::IntegrateAccel(float dt) {
	for (int i = 0; i < mDynamicObjectList.size(); i++) {
		PhysicsObject* object = mDynamicObjectList[i]->GetPhysicsObject();
		if (object == nullptr)
			continue;
		// inverse mass for multiplication instead of division and unmoving object
		float inverseMass = object->GetInverseMass();

		Vector3 linearVel = object->GetLinearVelocity();
		Vector3 force = object->GetForce();
		Vector3 accel = force * inverseMass;

		if (mApplyGravity && inverseMass > 0)
			accel += mGravity;

		linearVel += accel * dt;
		object->SetLinearVelocity(linearVel);

		// get objects current torque and angular velocity
		Vector3 torque = object->GetTorque();
		Vector3 angVel = object->GetAngularVelocity();

		// update objects orientation
		object->UpdateInertiaTensor();

		// get angular accel using new orientation * torque
		Vector3 angAccel = object->GetInertiaTensor() * torque;
		// scale by dt and set as new angular velocity
		angVel += angAccel * dt;
		object->SetAngularVelocity(angVel);
	}
}

/*
This function integrates linear and angular velocity into
position and orientation. It may be called multiple times
throughout a physics update, to slowly move the objects through
the world, looking for collisions.
*/
void PhysicsSystem::IntegrateVelocity(float dt) {
	float frameLinearDampening = 1.0f - (0.4f * dt);
	for (int i = 0; i < mDynamicObjectList.size(); i++) {
		PhysicsObject* object = mDynamicObjectList[i]->GetPhysicsObject();
		if (object == nullptr)
			continue;
		// determine position
		Transform& transform = mDynamicObjectList[i]->GetTransform();
		Vector3 position = transform.GetPosition();
		Vector3 linearVel = object->GetLinearVelocity();
		position += linearVel * dt;
		transform.SetPosition(position);
		// linear dampening
		linearVel = linearVel * frameLinearDampening;
		object->SetLinearVelocity(linearVel);

		// orientation
		Quaternion orientation = transform.GetOrientation();
		Vector3 angVel = object->GetAngularVelocity();
		orientation = orientation + (Quaternion(angVel * dt * 0.5f, 0.0f) * orientation);
		orientation.Normalise();
		transform.SetOrientation(orientation);

		// dampen new angular velocity
		float frameAngularDamping = 1.0f - (0.4f * dt);
		angVel = angVel * frameAngularDamping;
		object->SetAngularVelocity(angVel);
	}
}

/*
Once we're finished with a physics update, we have to
clear out any accumulated forces, ready to receive new
ones in the next 'game' frame.
*/
void PhysicsSystem::ClearForces() {
	mGameWorld.OperateOnContents(
		[](GameObject* o) {
			o->GetPhysicsObject()->ClearForces();
		}
	);
}


/*

As part of the final physics tutorials, we add in the ability
to constrain objects based on some extra calculation, allowing
us to model springs and ropes etc.

*/
void PhysicsSystem::UpdateConstraints(float dt) {
	std::vector<Constraint*>::const_iterator first;
	std::vector<Constraint*>::const_iterator last;
	mGameWorld.GetConstraintIterators(first, last);

	for (auto i = first; i != last; ++i) {
		(*i)->UpdateConstraint(dt);
	}
}