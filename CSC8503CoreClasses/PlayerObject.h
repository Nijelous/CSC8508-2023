#pragma once

#include "GameObject.h"
#include "../CSC8503/InventoryBuffSystem/InventoryBuffSystem.h"
#include "../CSC8503/SuspicionSystem/SuspicionSystem.h"
#include "UISystem.h"
#include "Interactable.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class PlayerObject : public GameObject, public PlayerBuffsObserver, public PlayerInventoryObserver {
		public:

			const enum PlayerSpeedState {
				Default,
				SpedUp,
				SlowedDown,
				Stunned
			};

			const enum AnnouncementType {
				DefaultAnnouncement,
				FlagAddedAnnouncement,
				FlagDroppedAnnouncement,
				CaughtByGuardAnnouncement
			};

			map<const AnnouncementType, const string> mAnnouncementTypeToStringMap{
				{DefaultAnnouncement, "DefaultAnnouncementText"},
				{FlagAddedAnnouncement,"The heist item was stolen by player : "},
				{FlagDroppedAnnouncement,"The heist item was dropped by player : "},
				{CaughtByGuardAnnouncement,"A guard has caught player : "}
			};

			PlayerObject(GameWorld* world,
				InventoryBuffSystem::InventoryBuffSystemClass* inventoryBuffSystemClassPtr,
				SuspicionSystem::SuspicionSystemClass* suspicionSystemClassptr,
				UISystem* UI, SoundObject* soundObject,
				const std::string& objName = "",
				int playerID = 0,int walkSpeed = 40, int sprintSpeed = 50, int crouchSpeed = 35, Vector3 offset = Vector3(0, 0, 0));

			PlayerObject(GameWorld* world,
				InventoryBuffSystem::InventoryBuffSystemClass* inventoryBuffSystemClassPtr,
				SuspicionSystem::SuspicionSystemClass* suspicionSystemClassptr,
				UISystem* UI,
				const std::string& objName = "",
				int playerID = 0, int walkSpeed = 40, int sprintSpeed = 50, int crouchSpeed = 35, Vector3 offset = Vector3(0, 0, 0));

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
			void UpdateGlobalUI(float dt);
			void UpdateLocalUI(float dt);
			void ShowDebugInfo(float dt);
			void ChangeActiveSusCausesBasedOnState(const GameObjectState &previousState, const GameObjectState& currentState);

			PlayerInventory::item GetEquippedItem();
			virtual void AddAnnouncement(AnnouncementType announcementType, float time, int playerNo) {
				const std::string annString = mAnnouncementTypeToStringMap[announcementType] + std::to_string(playerNo) + '!';
				mAnnouncementMap[annString] = time;
			}

			void SetUIObject(UISystem* ui) {
				mUi = ui;
			}

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

			GameWorld* mGameWorld;
			InventoryBuffSystem::InventoryBuffSystemClass* mInventoryBuffSystemClassPtr = nullptr;
			SuspicionSystem::SuspicionSystemClass* mSuspicionSystemClassPtr = nullptr;

			virtual void MovePlayer(float dt);
			
			virtual bool GotRaycastInput(NCL::CSC8503::InteractType &interactType,const float dt);
			virtual void RayCastFromPlayer(GameWorld* world,const NCL::CSC8503::InteractType& interactType,const float dt);
			virtual void RayCastFromPlayerForUI(GameWorld* world,const float dt);

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
			void    ChangeTransparency(bool isUp,float& transparency);
			void    RayCastIcon(GameObject* objectHit, float distance);
			void    ResetRayCastIcon();

			UISystem* mUi;
			float SusLinerInterpolation(float dt);

			float mSusValue=0.0;
			int mUiTime = 1;
			float tempSusValue = 0.0;
			float mLastSusValue = 0.0;
			float mAlarmTime=0.0;
			float mTransparencyRight = 0.0;
			float mTransparencyLeft = 0.0;
			float mTransparencyTop = 0.0;
			float mTransparencyBot = 0.0;
			float mTransparencyBotLeft = 0.0;
			float mTransparencyBotRight = 0.0;
			bool mIsDebugUIEnabled = false;
			float mTransparencyTopRight = 0.0;

			std::map<std::string , float> mAnnouncementMap;
			bool	IsSeenByGameObject(GameObject* otherGameObject);
		private:

		};
	}
}

