#ifdef USEGL
#pragma once
#include "GameObject.h"
#include "PlayerObject.h"
#include "../CSC8503/InventoryBuffSystem/InventoryBuffSystem.h"
#include "../CSC8503/SuspicionSystem/SuspicionSystem.h"
#include "Ray.h"

namespace NCL::CSC8503{
	class DebugNetworkedGame;
}

namespace NCL {
	namespace CSC8503 {
		class NetworkedGame;

		struct PlayerInputs{
			bool isSprinting = false;
			bool isCrouching = false;
			bool isEquippedItemUsed = false;
			bool isInteractButtonPressed = false;
			bool isHoldingInteractButton = false;

			int leftHandItemId = 0;
			int rightHandItemId = 0;
			
			bool movementButtons[4] = {false};

			float cameraYaw;

			Vector3 fwdAxis;
			Vector3 rightAxis;
			Ray rayFromPlayer;
		};
		
		class NetworkPlayer : public PlayerObject, public PlayerBuffsObserver, public PlayerInventoryObserver {
		public:
			NetworkPlayer(NetworkedGame* game, int num);
			NetworkPlayer(DebugNetworkedGame* game, int num, const std::string& objName);
			~NetworkPlayer();

			void OnCollisionBegin(GameObject* otherObject) override;

			void SetPlayerInput(const PlayerInputs& playerInputs);
			void SetIsLocalPlayer(bool isLocalPlayer);
			void SetCameraYaw(float cameraYaw);
			void ResetPlayerInput();
			void UpdateObject(float dt) override;
			void MovePlayer(float dt) override;
			void AddAnnouncement(AnnouncementType announcementType, float time, int playerNo) override;
			void SendAnnouncementPacket(AnnouncementType announcementType, float time, int playerNo);
			void SyncAnnouncements(AnnouncementType announcementType, float time, int playerNo);
			virtual void UpdatePlayerBuffsObserver(BuffEvent buffEvent, int playerNo) override 
			{ PlayerObject::UpdatePlayerBuffsObserver(buffEvent,playerNo); };
			virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved = false) override 
			{ PlayerObject::UpdateInventoryObserver(invEvent,playerNo, invSlot, isItemRemoved); };
			bool GetIsLocalPlayer() { return mIsLocalPlayer; };
		protected:
			bool mIsClientInputReceived = false;
			bool mIsLocalPlayer = false;

			//TODO(erendgrmc): set player camera start rotation. 
			float mCameraYaw = 0.f;
			
			DebugNetworkedGame* game;

			PlayerInputs mPlayerInputs;
			
			void HandleMovement(float dt, const PlayerInputs& playerInputs);
			bool GotRaycastInput(NCL::CSC8503::InteractType& interactType,const float dt) override;
			void RayCastFromPlayer(GameWorld* world, const NCL::CSC8503::InteractType& interactType, const float dt) override;

			void ControlInventory() override;
		};
	}
}

#endif