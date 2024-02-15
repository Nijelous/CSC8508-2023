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
            FlagGameObject() {};
            FlagGameObject(InventoryBuffSystemClass* inventoryBuffSystemClassPtr, std::map<GameObject*, int>* playerObjectToPlayerNoMap = nullptr);
            ~FlagGameObject();

            bool isServerPlayer(GameObject* otherObject);

            void GetFlag(int playerNo);
            void Reset();
            
            virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo) override;
            virtual void OnCollisionBegin(GameObject* otherObject) override;

        protected:
            std::map<GameObject*, int>* mPlayerObjectToPlayerNoMap;
            InventoryBuffSystemClass* mInventoryBuffSystemClassPtr;
        };
    }
}
