#pragma once

#include "GameObject.h"
#include "../CSC8503/InventoryBuffSystem/InventoryBuffSystem.h"
#include "../CSC8503/SuspicionSystem/SuspicionSystem.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class Interactable;

		enum PlayerState {
			Idle,
			Walk,
			Sprint,
			Crouch
		};

		enum PlayerSpeedState {
			Default,
			SpedUp,
			SlowedDown
		};

		class PlayerObject : public GameObject, public PlayerBuffsObserver, public PlayerInventoryObserver {
		public:
			PlayerObject(GameWorld* world, const std::string& objName = "",
				InventoryBuffSystem::InventoryBuffSystemClass* inventoryBuffSystemClassPtr = nullptr,
				SuspicionSystem::SuspicionSystemClass* suspicionSystemClassptr = nullptr,
				int playerID = 0,int walkSpeed = 40, int sprintSpeed = 50, int crouchSpeed = 35, Vector3 offset = Vector3(0, 0, 0));
			~PlayerObject();

			virtual void UpdateObject(float dt);
			virtual void UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo) override;
			virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo) override;

			PlayerInventory::item GetEquippedItem();

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

			PlayerState mPlayerState;
			PlayerSpeedState mPlayerSpeedState;

			GameWorld* mGameWorld;
			InventoryBuffSystem::InventoryBuffSystemClass* mInventoryBuffSystemClassPtr = nullptr;
			SuspicionSystem::SuspicionSystemClass* mSuspicionSystemClassPtr = nullptr;

			virtual void MovePlayer(float dt);
			
			void RayCastFromPlayer(GameWorld* world);

			void ControlInventory();

			void AttachCameraToPlayer(GameWorld* world);

			virtual void MatchCameraRotation(float yawValue);

			void StopSliding();

			void	ToggleCrouch(bool isCrouching);

			void	ActivateSprint(bool isSprinting);

			void	StartWalking();

			void	StartSprinting();

			void	StartCrouching();

			void	ChangeCharacterSize(float newSize);

			void	EnforceMaxSpeeds();
			void	EnforceSpedUpMaxSpeeds();
			void	ChangeToSlowedSpeeds();
			void	ChangeToDefaultSpeeds();
			void	ChangeToSpedUpSpeeds();
			void	UseItemForInteractable(Interactable* interactable);
		private:
		};
	}
}

