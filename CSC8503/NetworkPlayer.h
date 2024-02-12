#pragma once
#include "GameObject.h"
#include "GameClient.h"
#include "PlayerObject.h"

namespace NCL {
	namespace CSC8503 {
		class NetworkedGame;

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
		};
	}
}

