#pragma once
#include "GameWorld.h"
#include "MeshAnimation.h"



namespace NCL {
	class Maths::Vector3;
	class Maths::Vector4;
	namespace CSC8503 {
		class AnimationSystem {
		public:
			AnimationSystem(GameWorld& g);
			~AnimationSystem();

			void Clear();

			void Update(float dt);

			void GetAllAnimationObjects();

			void UpdateCurrentFrames(float dt);

			void UpdateMaterials();

			void UpdateAnimations();

			void PreloadAnimations();

		protected:


			GameWorld& gameWorld;
			vector<AnimationObject*> animationObjects;
			
		};
	}
}
