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
			AnimationSystem(GameWorld& g, std::map<std::string, MeshAnimation*>& preAnimationList);
			~AnimationSystem();

			void Clear();

			void Update(float dt, vector<GameObject*> updatableObjects);

			void UpdateAllAnimationObjects(float dt, vector<GameObject*> updatableObjects);

			void UpdateCurrentFrames(float dt);

			void SetGameObjectLists(vector<GameObject*> updatableObjects);

			void SetAnimationState(GameObject* gameObject, GameObject::GameObjectState objState);

		protected:
			GameWorld& gameWorld;
			vector<AnimationObject*> mAnimationList;
			vector<GuardObject*> mGuardList;
			vector<PlayerObject*> mPlayerList;
	
			Shader* mShader;
			Mesh* mMesh;
			MeshAnimation* mAnim;

			GameObject::GameObjectState mGuardState;
			GameObject::GameObjectState mPlayerState;
			std::map<std::string, MeshAnimation*>& mPreAnimationList;

			std::map<GameObject::GameObjectState, std::string> mGuardStateAnimationMap;
			std::map<GameObject::GameObjectState, std::string> mPlayerStateAnimationMap;

			void InitGuardStateAnimationMap();
			void InitPlayerStateAnimationMap();
		};
	}
}
#endif