#include "AnimationSystem.h"
#include "Camera.h"
#include "AnimationObject.h"


AnimationSystem::AnimationSystem(GameWorld& g):gameWorld(g)
{
	
}

AnimationSystem::~AnimationSystem()
{
}

void AnimationSystem::Clear()
{
	animationObjects.clear();
}

void AnimationSystem::Update(float dt)
{
	GetAllAnimationObjects();
	UpdateCurrentFrames(dt);

}

void AnimationSystem::GetAllAnimationObjects()
{	animationObjects.clear();

	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			if (o->IsActive()) {
				AnimationObject* animObj = o->GetAnimationObject();
				if (animObj) {
					animationObjects.emplace_back(animObj);
				}
			}
		}
	);
}

void AnimationSystem::UpdateCurrentFrames(float dt)
{
	for ( auto& a : animationObjects) {
		(*a).Update(dt);
		
	}
}

void AnimationSystem::UpdateAnimations()
{
	
	for (auto& a : animationObjects) {
		AnimationObject::mAnimationState state = (*a).GetAnimationState();
		switch (state)
		{
		case AnimationObject::mAnimationState::stand:
			break;
		case AnimationObject::mAnimationState::run:
			break;
		case AnimationObject::mAnimationState::jumpUp:
			break;
		case AnimationObject::mAnimationState::jumpDown:
			break;
		}
		
		
		
	}
	
}

void AnimationSystem::PreloadAnimations()
{
}
