#pragma once
#include "GameObject.h"
#include "Vector3.h"
#include "Interactable.h"

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class PointGameObject : public GameObject{
        public:
            PointGameObject(int pointsWorth = 0, float initCooldown=5);
            ~PointGameObject();

            void Activate(float dt);
            void Deactivate(float dt);
            void Waiting(float dt);
            virtual void UpdateObject(float dt);
   
            virtual void OnCollisionBegin(GameObject* otherObject) override;

            int GetPoints() { return mPoints; }
            void SetPoints(int points) {
                mPoints = points;
            }
            
        protected:
            int mPoints;
            StateMachine* mStateMachine;
            float mCooldown;
            float mInitCooldown;
        };
    }
}
