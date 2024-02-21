#pragma once
#include "GameObject.h"
#include "Vector3.h"
#include "StateMachine.h"
#include "InventoryBuffSystem.h"
#include "Item.h"
using namespace InventoryBuffSystem;

namespace NCL {
    namespace CSC8503 {
        class FlagGameObject : public Item {
        public:
            FlagGameObject(InventoryBuffSystemClass* inventoryBuffSystemClassPtr, std::map<GameObject*, int>* playerObjectToPlayerNoMap = nullptr, int pointsWorth = 0);
            ~FlagGameObject();

            bool isServerPlayer(GameObject* otherObject);

            void GetFlag(int playerNo);
            void Reset();

            void OnPlayerInteract(int playerId = 0) override;
   
            virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved = false) override;
            virtual void OnCollisionBegin(GameObject* otherObject) override;

            int GetPoints() { return mPoints; }
            void SetPoints(int points) {
                mPoints = points;
            }
            
        protected:
            std::map<GameObject*, int>* mPlayerObjectToPlayerNoMap;
            InventoryBuffSystemClass* mInventoryBuffSystemClassPtr;

            int mPoints;
        };
    }
}
