#pragma once
#include "GameObject.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include <string>
#include "../CSC8503/InventoryBuffSystem/PlayerBuffs.h"
#include "../Detour/Include/DetourNavMeshQuery.h"
using namespace std;
using namespace InventoryBuffSystem;

namespace NCL {
    namespace CSC8503 {
        class PlayerObject;
        constexpr int MIN_DIST_TO_NEXT_POS = 49;
        constexpr int GUARD_CATCHING_DISTANCE_SQUARED = 36;
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
            bool mIsBTWillBeExecuted;

            void BehaviourTree();
            void ExecuteBT();
            void MoveTowardFocalPoint(float* endPos);
            void LookTowardFocalPoint(Vector3 direction);
            void RunAfterPlayer(Vector3 direction);
            void GrabPlayer();
            float* QueryNavmesh(float* endPos);

            bool CheckPolyDistance();

            float mDist;
            float* mNextPoly = new float[3];
            float mConfiscateItemsTime;
            int mGuardSpeedMultiplier;
            float* mLastKnownPos = new float[3];

            BehaviourAction* Patrol();
            BehaviourAction* ChasePlayerSetup();
            BehaviourAction* GoToLastKnownLocation();
            BehaviourAction* ConfiscateItems();
            BehaviourAction* SendToPrison();

            BehaviourSequence* mRootSequence;
            BehaviourState mState = Ongoing;

            std::map<PlayerBuffs::buff, float> mAppliedBuffs;

        };
    }
}