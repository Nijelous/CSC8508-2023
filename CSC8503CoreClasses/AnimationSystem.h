#pragma once
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
			enum AnimationState {
                Stand,
                Walk,
                Sprint,
                Happy
            };
			AnimationSystem(GameWorld& g);
			~AnimationSystem();

			void Clear();

			void Update(float dt, vector<GameObject*> mUpdatableObjects , std::map<std::string, MeshAnimation*> preAnimationList);

			void UpdateAllAnimationObjects(float dt, vector<GameObject*> UpdatableObjects);

			void UpdateCurrentFrames(float dt);

			void UpdateAnimations(std::map<std::string, MeshAnimation*> preAnimationList);

			void PreloadMatTextures(GameTechRenderer& renderer);

			void SetGuardAnimationState(AnimationState animationState) {
				mGuardState = animationState;
			}

			AnimationState GetGuardAnimationState() {
				return mGuardState;
			}

			void SetPlayerAnimationState(AnimationState animationState) {
				mPlayerState = animationState;
			}

			AnimationState GetPlayerAnimationState() {
				return mPlayerState;
			}

		protected:


			GameWorld& gameWorld;
			vector<AnimationObject*> mAnimationList;
			vector<GuardObject*> mGuardList;
			vector<PlayerObject*> mPlayerList;
			vector<GLuint>  mMatTextures;
			Shader* mShader;
			Mesh* mMesh;
			MeshAnimation* mAnim;
			Texture* mAnimTexture = nullptr;	

			AnimationState mGuardState;
			AnimationState mPlayerState;
		};
	}
}
