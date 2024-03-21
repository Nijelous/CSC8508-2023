#pragma once

#ifdef USEGL

#include "GameWorld.h"
#include "MeshAnimation.h"
#include "Mesh.h"


namespace NCL {
	class Maths::Vector3;
	class Maths::Vector4;
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

			void SetGameObjectLists(vector<GameObject*> updatableObjects);

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
#endif