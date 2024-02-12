#pragma once
#include "GameWorld.h"
#include "MeshAnimation.h"
#include "RenderObject.h"
#include "OGLRenderer.h"
#include "OGLTexture.h"
#include "../CSC8503/GameTechRenderer.h"


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

			void PreloadMatTextures();

		protected:


			GameWorld& gameWorld;
			vector<AnimationObject*> animationList;
			vector<GLuint>  mMatTextures;
		};
	}
}
