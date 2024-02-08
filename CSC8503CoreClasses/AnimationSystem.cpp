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
	animationList.clear();
}

void AnimationSystem::Update(float dt)
{
	
	GetAllAnimationObjects();
	UpdateCurrentFrames(dt);


}

void AnimationSystem::GetAllAnimationObjects()
{	animationList.clear();

	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			if (o->GetAnimationObject()) {
				AnimationObject* animObj = o->GetAnimationObject();
				// if (animObj) {
					animationList.emplace_back(animObj);
					
					o->GetRenderObject()->SetAnimation(o->GetAnimationObject()->GetAnimation());
					o->GetRenderObject()->SetMaterial(o->GetAnimationObject()->GetMaterial());
					o->GetRenderObject()->SetCurrentFrame(o->GetAnimationObject()->GetCurrentFrame());
				//std::cout << animObj << std::endl;
				// }
			}
		}
	);
}

void AnimationSystem::UpdateCurrentFrames(float dt)
{
	for ( auto& a : animationList) {
		(*a).Update(dt);
	}
}

void AnimationSystem::UpdateAnimations()
{
	
	for (auto& a : animationList) {
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
