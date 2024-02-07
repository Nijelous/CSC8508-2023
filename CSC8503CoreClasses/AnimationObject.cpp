#include "AnimationObject.h"

using namespace NCL;
using namespace CSC8503;
AnimationObject::AnimationObject(MeshAnimation* animation, MeshMaterial* material){
	this->mAnimation = animation;
	this->mMaterial = material;


	mIsAnimation = true;
	mState = stand;
	mCurrentFrame = 0;
	mNextFrame = 0;
	mFrameTime = 0.0f;
}

AnimationObject::~AnimationObject() {
	delete mAnimation;
	delete mMaterial;
	
}

void AnimationObject::Update(float dt){
	mFrameTime -= dt;
	std::cout << mFrameTime << std::endl;
	while (mFrameTime < 0.0f) {
		mCurrentFrame = (mCurrentFrame + 1) % mAnimation->GetFrameCount();
		mNextFrame = (mCurrentFrame + 1) % mAnimation->GetFrameCount();
		mFrameTime += 1.0f/mAnimation->GetFrameRate();
		//std::cout << currentFrame << std::endl; 
	}
}

