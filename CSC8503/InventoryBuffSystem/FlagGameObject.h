#pragma once
#include "GameObject.h"
#include "Vector3.h"
#include "StateMachine.h"
#include "InventoryBuffSystem.h"
#include "Item.h"
#include "NetworkObject.h"
#include "../SuspicionSystem/SuspicionSystem.h"
using namespace InventoryBuffSystem;
using namespace SuspicionSystem;

namespace NCL {
    namespace CSC8503 {
        class FlagGameObject : public Item, public PlayerBuffsObserver{
        public:
            FlagGameObject(InventoryBuffSystemClass* inventoryBuffSystemClassPtr, SuspicionSystemClass* suspicionSystemClassPtr, std::map<GameObject*, int>* playerObjectToPlayerNoMap = nullptr, int pointsWorth = 0);
            ~FlagGameObject();

            void GetFlag(int playerNo);
            void Reset();

            void OnPlayerInteract(int playerId = 0) override;
   
            virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved = false) override;
            virtual void OnCollisionBegin(GameObject* otherObject) override;

            int GetPoints() { return mPoints; }
            void SetPoints(int points) {
                mPoints = points;
            }
            const bool IsMultiplayerAndIsNotServer();
        protected:
            void UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo) override;

            std::map<GameObject*, int>* mPlayerObjectToPlayerNoMap;
            InventoryBuffSystemClass* mInventoryBuffSystemClassPtr;
            SuspicionSystemClass* mSuspicionSystemClassPtr;
            int mPoints;
        };
    }
}