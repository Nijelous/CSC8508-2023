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

			void Update(float dt, vector<GameObject*> mUpdatableObjects , std::map<std::string, MeshAnimation*> preAnimationList);

			void UpdateAllAnimationObjects(float dt, vector<GameObject*> UpdatableObjects);

			void UpdateCurrentFrames(float dt);

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
