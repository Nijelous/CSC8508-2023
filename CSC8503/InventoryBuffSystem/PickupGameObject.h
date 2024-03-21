#pragma once
#include "GameObject.h"
#include "Vector3.h"
#include "PlayerBuffs.h"
#include "PlayerInventory.h"
#include "InventoryBuffSystem.h"
#include "map"

using namespace InventoryBuffSystem;

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class PickupGameObject : public GameObject {
        public:
            PickupGameObject() {};
            PickupGameObject(
                InventoryBuffSystemClass* inventoryBuffSystemClassPtr,
                bool isMultiplayer,
                unsigned int randomSeed = 10,
                std::map<GameObject*, int>* playerObjectToPlayerNoMap = nullptr,
                float initCooldown = 50.0f);
            ~PickupGameObject();

            virtual void UpdateObject(float dt);

            void ChangeToRandomPickup();
            void ActivatePickup(int playerNo);

            virtual void OnCollisionBegin(GameObject* otherObject) override;

            bool IsBuff() const { return mIsBuff; }

            PlayerInventory::item GetItem() const { return mCurrentItem; }
            PlayerBuffs::buff GetBuff() const { return mCurrentBuff; }

        protected:
            void Activate(float dt); //go over the surface
            void Deactivate(float dt); // go under the surface
            void Waiting(float dt); // wait to go over

            StateMachine* mStateMachine;
            float mCooldown;
            float mInitCooldown;
            std::map<GameObject* ,int>* mPlayerObjectToPlayerNoMap;

            unsigned int mRandomSeed;

            bool mIsBuff;
            bool mIsMultiplayer;
            InventoryBuffSystemClass* mInventoryBuffSystemClassPtr;
            PlayerInventory::item mCurrentItem;
            PlayerBuffs::buff mCurrentBuff;
        };
    }
}
