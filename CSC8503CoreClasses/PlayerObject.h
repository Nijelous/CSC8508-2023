#pragma once

#include "GameObject.h"
#include "../CSC8503/InventoryBuffSystem/InventoryBuffSystem.h"
#include "../CSC8503/SuspicionSystem/SuspicionSystem.h"
#include "UISystem.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class Interactable;
		class PlayerObject : public GameObject, public PlayerBuffsObserver, public PlayerInventoryObserver {
		public:

			enum PlayerSpeedState {
				Default,
				SpedUp,
				SlowedDown,
				Stunned
			};

			PlayerObject(GameWorld* world,
				InventoryBuffSystem::InventoryBuffSystemClass* inventoryBuffSystemClassPtr,
				SuspicionSystem::SuspicionSystemClass* suspicionSystemClassptr,
				UISystem* UI, SoundObject* soundObject,
				const std::string& objName = "",PrisonDoor* prisonDoorPtr = nullptr,
				int playerID = 0,int walkSpeed = 40, int sprintSpeed = 50, int crouchSpeed = 35, Vector3 offset = Vector3(0, 0, 0));
			~PlayerObject();

			int GetPlayerID() const {
				return mPlayerID;
			}

			int GetActiveItemSlot() const {
				return mActiveItemSlot;
			}

			int GetPoints() { return mPlayerPoints; }
			void ResetPlayerPoints() { mPlayerPoints = 0; }
			void AddPlayerPoints(int addedPoints) { mPlayerPoints += addedPoints; }
			virtual void UpdateObject(float dt);
			virtual void UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo) override;
			virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved = false) override;

			PlayerInventory::item GetEquippedItem();

			void ClosePrisonDoor();

			void SetUIObject(UISystem* ui) {
				mUi = ui;
			}

			void SetPrisonDoor(PrisonDoor* prisonDoor);

		protected:
			bool mIsCrouched;

			int mMovementSpeed;
			int mWalkSpeed;
			int mSprintSpeed;
			int mCrouchSpeed;
			int mActiveItemSlot;

			int mPlayerID;
			float mInteractHeldDt;
			bool mHasSilentSprintBuff;

			int mPlayerPoints;

			PlayerSpeedState mPlayerSpeedState;
			PrisonDoor* mPrisonDoorPtr;

			GameWorld* mGameWorld;
			InventoryBuffSystem::InventoryBuffSystemClass* mInventoryBuffSystemClassPtr = nullptr;
			SuspicionSystem::SuspicionSystemClass* mSuspicionSystemClassPtr = nullptr;

			virtual void MovePlayer(float dt);
			
			virtual void RayCastFromPlayer(GameWorld* world, float dt);

			virtual void ControlInventory();

			void AttachCameraToPlayer(GameWorld* world);

			virtual void MatchCameraRotation(float yawValue);

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

			UISystem* mUi;
			float SusLinerInterpolation(float dt);

			float mSusValue=0.0;
			int mUiTime = 1;
			float tempSusValue = 0.0;
			float mLastSusValue = 0.0;
			float mAlarmTime=0.0;
			float transparency1 = 0.0;
			float transparency2 = 0.0;
			

			bool	IsSeenByGameObject(GameObject* otherGameObject);
		private:

		};
	}
}

