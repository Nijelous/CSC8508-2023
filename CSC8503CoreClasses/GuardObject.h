#pragma once
#include "GameObject.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include <string>
#include "../CSC8503/InventoryBuffSystem/PlayerBuffs.h"
using namespace std;
using namespace InventoryBuffSystem;

namespace NCL {
    namespace CSC8503 {
        class PlayerObject;
        class GuardObject : public GameObject {
        public:
            GuardObject(const std::string& name = "");
            ~GuardObject();

            void UpdateObject(float dt) override;
            void ApplyBuffToGuard(PlayerBuffs::buff buffToApply);
            void RemoveBuffFromGuard(PlayerBuffs::buff removedBuff);

            void SetPlayer(PlayerObject* newPlayer) {
                mPlayer = newPlayer;
            }

            void SetPatrolNodes(vector<Vector3> nodes) {
                mNodes = nodes;
            }

            void SetCurrentNode(int node) {
                mCurrentNode = node;
            }

        protected:
            void RaycastToPlayer();
            Vector3 GuardForwardVector();
            float AngleFromFocalPoint(Vector3 direction);
            void HandleAppliedBuffs(float dt);

            GameObject* mSightedObject;
            PlayerObject* mPlayer;

            vector<Vector3> mNodes;
            int mCurrentNode;
            int mNextNode;
        private:
            bool mCanSeePlayer;
            bool mHasCaughtPlayer;
            bool mPlayerHasItems;
            bool mIsStunned;

            void BehaviourTree();
            void ExecuteBT();
            void MoveTowardFocalPoint(Vector3 direction);
            void LookTowardFocalPoint(Vector3 direction);
            void GrabPlayer();
            void QueryNavmesh();

            float mConfiscateItemsTime;
            int mGuardSpeedMultiplier;

            BehaviourAction* Patrol();
            BehaviourAction* ChasePlayerSetup();
            BehaviourAction* ConfiscateItems();
            BehaviourAction* SendToPrison();

            BehaviourSequence* mRootSequence;
            BehaviourState mState = Ongoing;

            std::map<PlayerBuffs::buff, float> mAppliedBuffs;

        };
    }
}