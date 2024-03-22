#pragma once

#include "GameWorld.h"
#include "MeshAnimation.h"
#include "RenderObject.h"

#ifdef USEGL
#include "../CSC8503/GameTechRenderer.h"
#include "OGLRenderer.h"
#include "OGLTexture.h"
#endif

#ifdef USEPROSPERO
#include "../CSC8503/GameTechAGCRenderer.h"
#include "AGCRenderer.h"
#include "AGCTexture.h"
#endif


#include "GuardObject.h"
#include "PlayerObject.h"
#include "Mesh.h"


namespace NCL {
	namespace CSC8503 {
		class AnimationObject;
		class PlayerObject;
		class GuardObject;
		class AnimationSystem {
		public:
			AnimationSystem(GameWorld& g, std::map<std::string, NCL::Rendering::MeshAnimation*>& preAnimationList);
			~AnimationSystem();

			void Clear();

			void Update(float dt, vector<GameObject*> updatableObjects);

			void UpdateAllAnimationObjects(float dt, vector<GameObject*> updatableObjects);

			void UpdateCurrentFrames(float dt);

			void UpdateAnimations(std::map<std::string, MeshAnimation*> preAnimationList);

			void SetGameObjectLists(vector<GameObject*> UpdatableObjects);

			void SetAnimationState(GameObject* gameObject, GameObject::GameObjectState objState);

		protected:
			GameWorld& gameWorld;
			vector<AnimationObject*> mAnimationList;
			vector<GuardObject*> mGuardList;
			vector<PlayerObject*> mPlayerList;

			NCL::Rendering::Mesh* mMesh;
			NCL::Rendering::MeshAnimation* mAnim;

			GameObject::GameObjectState mGuardState;
			GameObject::GameObjectState mPlayerState;
			std::map<std::string, NCL::Rendering::MeshAnimation*>& mPreAnimationList;

			std::map<GameObject::GameObjectState, std::string> mGuardStateAnimationMap;
			std::map<GameObject::GameObjectState, std::string> mPlayerStateAnimationMap;

			void InitGuardStateAnimationMap();
			void InitPlayerStateAnimationMap();
		};
	}
}