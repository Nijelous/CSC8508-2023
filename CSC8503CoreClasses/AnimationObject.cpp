#include "AnimationObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"



AnimationObject::AnimationObject(const std::string& objectName) {
	name = objectName;
	worldID = -1;
	isActive = true;
	boundingVolume = nullptr;
	physicsObject = nullptr;
	renderObject = nullptr;
	networkObject = nullptr;
	animation = nullptr;
	material = nullptr;
	
	
}

AnimationObject::~AnimationObject() {
	delete boundingVolume;
	delete physicsObject;
	delete renderObject;
	delete networkObject;
	delete animation;
	delete material;
}

void AnimationObject::setMeshAnimationRun() {
	NCL::Rendering::MeshAnimation* animation = new MeshAnimation("Role_T.anm");
	//I am not sure is it the best way to handle the error about different namespace!
	
}
