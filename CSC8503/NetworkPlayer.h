#pragma once
#include "GameObject.h"
#include "PlayerObject.h"

namespace NCL {
	namespace CSC8503 {
		class NetworkedGame;

		struct PlayerInputs{
			bool isSprinting = false;
			bool isCrouching = false;
			bool isLeftHandUsed = false;
			bool isRightHandUsed = false;

			int leftHandItemId = 0;
			int rightHandItemId = 0;
			
			bool movementButtons[4] = {false};
		};
		
		class NetworkPlayer : public PlayerObject{
		public:
			NetworkPlayer(NetworkedGame* game, int num);
			NetworkPlayer(NetworkedGame* game, int num, const std::string& objName);
			~NetworkPlayer();

			void OnCollisionBegin(GameObject* otherObject) override;

			void MovePlayer(float dt) override;

			int GetPlayerNum() const {
				return playerNum;
			}

		protected:
			NetworkedGame* game;
			int playerNum;
			
			void HandleMovement(float dt, const PlayerInputs& playerInputs);
			
		};
	}
}

