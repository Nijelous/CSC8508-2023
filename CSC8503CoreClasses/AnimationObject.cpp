#include "AnimationObject.h"

using namespace NCL;
using namespace CSC8503;
AnimationObject::AnimationObject(AnimationType animationType,MeshAnimation* animation){
	this->mAnimation = animation;
	this->mAnimationType = animationType;

	mCurrentFrame = 0;
	mNextFrame = 0;
	mFrameTime = 0.0f;
	mRate = 1.0;
}

AnimationObject::~AnimationObject() {
	
	
}

void AnimationObject::Update(float dt){
	if (mAnimation == nullptr) return;
	mFrameTime -= mRate*dt;

	while (mFrameTime < 0.0f) {
		mCurrentFrame = (mCurrentFrame + 1) % mAnimation->GetFrameCount();
		mNextFrame = (mCurrentFrame + 1) % mAnimation->GetFrameCount();
		mFrameTime += 1.0f/mAnimation->GetFrameRate();
	}
}

