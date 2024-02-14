#include "AnimationObject.h"

using namespace NCL;
using namespace CSC8503;
AnimationObject::AnimationObject(MeshAnimation* animation, MeshMaterial* material){
	this->mAnimation = animation;
	this->mMaterial = material;

	mState = Stand;
	mCurrentFrame = 0;
	mNextFrame = 0;
	mFrameTime = 0.0f;
}

AnimationObject::~AnimationObject() {
	
	
}

void AnimationObject::Update(float dt){
	mFrameTime -= 2*dt;

	while (mFrameTime < 0.0f) {
		mCurrentFrame = (mCurrentFrame + 1) % mAnimation->GetFrameCount();
		mNextFrame = (mCurrentFrame + 1) % mAnimation->GetFrameCount();
		mFrameTime += 1.0f/mAnimation->GetFrameRate();
		//std::cout << currentFrame << std::endl; 
	}
}

