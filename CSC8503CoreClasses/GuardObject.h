#pragma once
#include "GameObject.h"
#include "GameWorld.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include <string>
using namespace std;


namespace NCL {
    namespace CSC8503 {
        class GuardObject : public GameObject {
        public:
            GuardObject(const std::string& name = "");
            ~GuardObject();

            virtual void UpdateObject(float dt) override;

            void SetPlayer(GameObject* newPlayer) {
                mPlayer = newPlayer;
            }

            void SetGameWorld(GameWorld* newWorld) {
                mWorld = newWorld;
            }
        protected:
            void RaycastToPlayer();
            Vector3 AngleOfSight();

            GameObject* mSightedObject;
            GameObject* mPlayer;
            GameWorld* mWorld;
        private:
            bool mCanSeePlayer;
            bool mHasCaughtPlayer;
            bool mHasConfiscatedItems;

            void BehaviourTree();
            void ExecuteBT();

            float mTimer;

            BehaviourAction* Patrol();
            BehaviourAction* ChasePlayerSetup();
            BehaviourAction* ConfiscateItems();
            BehaviourAction* SendToPrison();

            BehaviourSequence* mRootSequence;
            BehaviourState mState = Ongoing;
        };
    }
}