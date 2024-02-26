#pragma once

#include "GameObject.h"
#include "../CSC8503/InventoryBuffSystem/InventoryBuffSystem.h"
#include "../CSC8503/SuspicionSystem/SuspicionSystem.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class Interactable;
		class PlayerObject : public GameObject, public PlayerBuffsObserver, public PlayerInventoryObserver {
		public:
			enum PlayerState {
				Stand,
				Walk,
				Sprint,
				Crouch,
				Happy
			};

			enum PlayerSpeedState {
				Default,
				SpedUp,
				SlowedDown,
				Stunned
			};

			PlayerObject(GameWorld* world, const std::string& objName = "",
				InventoryBuffSystem::InventoryBuffSystemClass* inventoryBuffSystemClassPtr = nullptr,
				SuspicionSystem::SuspicionSystemClass* suspicionSystemClassptr = nullptr, PrisonDoor* prisonDoorPtr = nullptr,
				int playerID = 0,int walkSpeed = 40, int sprintSpeed = 50, int crouchSpeed = 35, Vector3 offset = Vector3(0, 0, 0));
			~PlayerObject();


			int GetPoints() { return mPlayerPoints; }
			void ResetPlayerPoints() { mPlayerPoints = 0; }
			void AddPlayerPoints(int addedPoints) { mPlayerPoints += addedPoints; }
			virtual void UpdateObject(float dt);
			virtual void UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo) override;
			virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved = false) override;

			PlayerInventory::item GetEquippedItem();

			void ClosePrisonDoor();

			PlayerState GetPlayerState() { return mObjectState; };

		protected:
			bool mIsCrouched;

			int mMovementSpeed;
			int mWalkSpeed;
			int mSprintSpeed;
			int mCrouchSpeed;
			int mActiveItemSlot;

			int mPlayerNo;
			float mInteractHeldDt;
			bool mHasSilentSprintBuff;
			int mFirstInventorySlotUsageCount;
			int mSecondInventorySlotUsageCount;

			int mPlayerPoints;

			PlayerState mObjectState;
			PlayerSpeedState mPlayerSpeedState;
			PrisonDoor* mPrisonDoorPtr;

			GameWorld* mGameWorld;
			InventoryBuffSystem::InventoryBuffSystemClass* mInventoryBuffSystemClassPtr = nullptr;
			SuspicionSystem::SuspicionSystemClass* mSuspicionSystemClassPtr = nullptr;

			virtual void MovePlayer(float dt);

			virtual void OnPlayerUseItem();
			
			void RayCastFromPlayer(GameWorld* world, float dt);

			void ControlInventory();

			void AttachCameraToPlayer(GameWorld* world);

			virtual void MatchCameraRotation(float yawValue);

			void ResetEquippedItemUsageCount(int inventorySlot);

			void StopSliding();

			void	ToggleCrouch(bool isCrouching);

			void	ActivateSprint(bool isSprinting);

			void	StartWalking();

			void	StartSprinting();

			void	StartCrouching();

			void	ChangeCharacterSize(float newSize);

			void	EnforceSpedUpMaxSpeeds();
			void	ChangeToSlowedSpeeds();
			void	ChangeToDefaultSpeeds();
			void	ChangeToSpedUpSpeeds();
			void	ChangeToStunned();
			void	UseItemForInteractable(Interactable* interactable);

			void	EnforceMaxSpeeds();
		private:
		};
	}
}

