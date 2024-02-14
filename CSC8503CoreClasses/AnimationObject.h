#pragma once

#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"
#include "MeshAnimation.h"
#include "MeshMaterial.h"





namespace NCL {
    using namespace NCL::Rendering;

    namespace CSC8503 {
        class Transform;
        class Maths::Vector3;
        class Maths::Vector4;
        class AnimationObject
        {
        public:
            enum mAnimationState {
                Stand,
                Walk,
                Sprint,
                Happy
             };

            AnimationObject(MeshAnimation* animation, MeshMaterial* material);
            ~AnimationObject();

            void Update(float dt);

            void SetAnimation(MeshAnimation* animation){
                mAnimation = animation;
            }

            void SetMaterial(MeshMaterial* material){
                mMaterial = material;
            }

            MeshAnimation* GetAnimation() {
                return mAnimation;
            }

            MeshMaterial* GetMaterial() {
                return mMaterial;
            }

            int	GetCurrentFrame() {
                return mCurrentFrame;
            }
           
            void SetAnimationState(mAnimationState animationState) {
                mState = animationState;
            }

            mAnimationState GetAnimationState() {
                return mState;
            }
            
            
        protected:
            MeshAnimation* mAnimation;
            MeshMaterial* mMaterial;
            mAnimationState mState;
           
            int		mCurrentFrame;
            int		mNextFrame;
            float	mFrameTime;
           
        };
    }
}
