#pragma once

#ifdef USEGL

#include "GameWorld.h"
#include "MeshAnimation.h"
#include "RenderObject.h"
#include "OGLRenderer.h"
#include "OGLTexture.h"
#include "../CSC8503/GameTechRenderer.h"
#include "GuardObject.h"
#include "PlayerObject.h"


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

			void PreloadMatTextures(GameTechRenderer& renderer, Mesh& mesh, MeshMaterial& meshMaterial, vector<GLuint>& matTextures);


			void SetGameObjectLists(vector<GameObject*> UpdatableObjects, vector<GLuint> mRigTexture, vector<GLuint>& mGuardTextures);


		protected:


			GameWorld& gameWorld;
			vector<AnimationObject*> mAnimationList;
			vector<GuardObject*> mGuardList;
			vector<PlayerObject*> mPlayerList;
	
			Shader* mShader;
			Mesh* mMesh;
			MeshAnimation* mAnim;
			Texture* mAnimTexture;	

			GameObject::GameObjectState mGuardState;
			GameObject::GameObjectState mPlayerState;
		};
	}
}
#endif