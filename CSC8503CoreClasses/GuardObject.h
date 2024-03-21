//Created by Oliver Perrin
//Edited by Eren Degirmenci 

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
        constexpr int MIN_DIST_TO_NEXT_POS = 49;
        constexpr int GUARD_CATCHING_DISTANCE_SQUARED = 36;
        constexpr float FUMBLE_KEYS_TIME = 0.15;
        constexpr float RAYCAST_INTERVAL = 0.1;
        constexpr int HIGH_SUSPICION = 70;
        constexpr int MAX_DIST_TO_SUS_LOCATION = 6400;
        constexpr float POINTING_TIMER = 120;
        constexpr int MAX_NUMBER_OF_FRAMES_GUARD_STUCK = 200;
      
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

            void AddPlayer(PlayerObject* player){
                mPlayerList.push_back(player);
            }

        protected:
            void RaycastToPlayer();
            Vector3 GuardForwardVector();
            float AngleFromFocalPoint(Vector3 direction);
            void HandleAppliedBuffs(float dt);
            PlayerObject* GetPlayerToChase();

            GameObject* mSightedPlayer;
            GameObject* mSightedDoor;
            PlayerObject* mPlayer;
            std::vector<PlayerObject*> mPlayerList;

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
            void GuardSpeedMultiplier();
            int AngleValue(float minAng);
            bool IsHighEnoughLocationSus();
            bool IsPlayerSprintingNearby();
            void DebugMode();

            void CheckForDoors(float dt);
            void OpenDoor();
            void SendAnnouncementToPlayer();
            float mDist;
            float* mNextPoly = new float[3];
            float mConfiscateItemsTime;
            int mGuardSpeedMultiplier;
            float* mLastKnownPos = new float[3];
            float mDoorRaycastInterval;
            float mFumbleKeysCurrentTime;
            float mPointTimer;
            float mSmallestDistance;
            Vector3 mSmallestDistanceVector;
            Vector3* mNearestSprintingPlayerDir;
            float mLastDist;
            int mDistCounter;
            bool mDebugMode;

            BehaviourAction* Patrol();
            BehaviourAction* CheckSusLocation();
            BehaviourAction* PointAtPlayer();
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