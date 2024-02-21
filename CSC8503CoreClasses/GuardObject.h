#pragma once
#include "GameObject.h"
#include "GameWorld.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include <string>
using namespace std;


namespace NCL {
    namespace CSC8503 {
        class PlayerObject;
        class GuardObject : public GameObject {
        public:
            GuardObject(const std::string& name = "");
            ~GuardObject();

            enum GuardState {
                Stand,
                Walk,
                Sprint,
                Happy
            };


            virtual void UpdateObject(float dt) override;

            void SetPlayer(PlayerObject* newPlayer) {
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

            void SetCurrentNode(int node) {
                mCurrentNode = node;
            }

            GuardState GetGuardState() {
                return  mGuardState;
            }
        protected:
            void RaycastToPlayer();
            Vector3 GuardForwardVector();
            float AngleFromFocalPoint(Vector3 direction);

            GameObject* mSightedObject;
            PlayerObject* mPlayer;
            const GameWorld* mWorld;
            Vector3 mPrisonPosition;
            vector<Vector3> mNodes;
            int mCurrentNode;
            int mNextNode;
        private:
            bool mCanSeePlayer;
            bool mHasCaughtPlayer;
            bool mPlayerHasItems;

            void BehaviourTree();
            void ExecuteBT();
            void MoveTowardFocalPoint(Vector3 direction);
            void LookTowardFocalPoint(Vector3 direction);
            void GrabPlayer();

            float mConfiscateItemsTime;
            int mGuardSpeedMultiplier;

            BehaviourAction* Patrol();
            BehaviourAction* ChasePlayerSetup();
            BehaviourAction* ConfiscateItems();
            BehaviourAction* SendToPrison();

            BehaviourSequence* mRootSequence;
            BehaviourState mState = Ongoing;

            GuardState mGuardState;
        };
    }
}