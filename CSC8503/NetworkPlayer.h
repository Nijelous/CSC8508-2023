#pragma once
#include "GameObject.h"
#include "PlayerObject.h"
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
		
		class NetworkPlayer : public PlayerObject{
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

		protected:
			bool mIsClientInputReceived = false;
			bool mIsLocalPlayer = false;

			//TODO(erendgrmc): set player camera start rotation. 
			float mCameraYaw = 0.f;
			
			DebugNetworkedGame* game;

			PlayerInputs mPlayerInputs;
			
			void HandleMovement(float dt, const PlayerInputs& playerInputs);

			void OnPlayerUseItem() override;

			void RayCastFromPlayer(GameWorld* world, float dt) override;

			void ControlInventory() override;
		};
	}
}

