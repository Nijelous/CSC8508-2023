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
            enum AnimationType {
                playerAnimation,
                guardAnimation
            };

            AnimationObject(AnimationType animationType,MeshAnimation* animation, MeshMaterial* material);
            ~AnimationObject();

            void Update(float dt);

            void SetAnimation(MeshAnimation* animation) {
                mAnimation = animation;
            }

            void SetMaterial(MeshMaterial* material) {
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

            void ReSetCurrentFrame() {
                mCurrentFrame = 0;
            }
            
            void SetRate(float rate) {
                mRate = rate;
            }

            float GetRate() {
                return mRate;
            }

            AnimationType GetAnimationType() {
                return mAnimationType;
            }
           
            
        protected:
            MeshAnimation* mAnimation;
            MeshMaterial* mMaterial;
            AnimationType mAnimationType;
           
            int		mCurrentFrame;
            int		mNextFrame;
            float	mFrameTime;
            float		mRate;
           
        };
    }
}
