#pragma once

#include "MeshAnimation.h"
#include "MeshMaterial.h"

namespace NCL {
    using namespace NCL::Rendering;

    namespace CSC8503 {
        class Transform;

        class AnimationObject
        {
        public:
            enum AnimationType {
                playerAnimation,
                guardAnimation
            };

            AnimationObject(AnimationType animationType,MeshAnimation* animation);
            ~AnimationObject();

            void Update(float dt);

            void SetAnimation(MeshAnimation* animation) {
                mAnimation = animation;
            }

            MeshAnimation* GetAnimation() {
                return mAnimation;
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
            AnimationType mAnimationType;
           
            int		mCurrentFrame;
            int		mNextFrame;
            float	mFrameTime;
            float		mRate;
           
        };
    }
}
