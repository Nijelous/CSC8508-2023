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

            void SetPrisonPosition(Vector3 prisonPosition) {
                mPrisonPosition = prisonPosition;
            }

            void SetPatrolNodes(vector<Vector3> nodes) {
                mNodes = nodes;
            }
        protected:
            void RaycastToPlayer();
            Vector3 GuardForwardVector();
            void SetFocus();

            GameObject* mSightedObject;
            GameObject* mPlayer;
            GameWorld* mWorld;
            Vector3 mPrisonPosition;
            vector<Vector3> mNodes;
        private:
            bool mCanSeePlayer;
            bool mHasCaughtPlayer;
            bool mPlayerHasItems;

            void BehaviourTree();
            void ExecuteBT();

            float mConfiscateItemsTime;

            BehaviourAction* Patrol();
            BehaviourAction* ChasePlayerSetup();
            BehaviourAction* ConfiscateItems();
            BehaviourAction* SendToPrison();

            BehaviourSequence* mRootSequence;
            BehaviourState mState = Ongoing;
        };
    }
}