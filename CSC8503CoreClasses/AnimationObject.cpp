#include "AnimationObject.h"

using namespace NCL;
using namespace CSC8503;
AnimationObject::AnimationObject(MeshAnimation* animation, MeshMaterial* material){
	this->mAnimation = animation;
	this->mMaterial = material;


	mIsAnimation = true;
	mState = stand;
	currentFrame = 0;
	nextFrame = 0;
	frameTime = 0.0f;
}

AnimationObject::~AnimationObject() {
	delete mAnimation;
	delete mMaterial;
	
}

void AnimationObject::Update(float dt){
	frameTime -= dt;
	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % mAnimation->GetFrameCount();
		nextFrame = (currentFrame + 1) % mAnimation->GetFrameCount();
		frameTime += mAnimation->GetFrameTime();
		//std::cout << currentFrame << std::endl; test
	}
}

