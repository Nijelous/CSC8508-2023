#pragma once
#include "GameWorld.h"
#include "MeshAnimation.h"
#include "RenderObject.h"
#include "OGLRenderer.h"
#include "OGLTexture.h"
#include "../CSC8503/GameTechRenderer.h"
#include "GuardObject.h"


namespace NCL {
	class Maths::Vector3;
	class Maths::Vector4;
	namespace CSC8503 {
		class AnimationSystem {
		public:
			AnimationSystem(GameWorld& g);
			~AnimationSystem();

			void Clear();

			void Update(float dt, std::map<std::string, MeshAnimation*> preAnimationList);

			void UpdateAllAnimationObjects(float dt);

			void UpdateCurrentFrames(float dt);

			void UpdateMaterials();

			void UpdateAnimations(std::map<std::string, MeshAnimation*> preAnimationList);

			void PreloadMatTextures(GameTechRenderer* renderer);

		protected:


			GameWorld& gameWorld;
			vector<AnimationObject*> mAnimationList;
			vector<GuardObject*> mGuardList;
			vector<GLuint>  mMatTextures;
			Shader* mShader;
			Mesh* mMesh;
			MeshAnimation* mAnim;
			Texture* mAnimTexture = nullptr;	
		};
	}
}
