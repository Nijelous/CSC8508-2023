#pragma once

#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;

		class PlayerObject : public GameObject {
		public:
					PlayerObject(GameWorld* world, const std::string& objName = "", int movementSpeed = 500);
					~PlayerObject();

			void	UpdateObject(float dt);

		protected:

            virtual void MovePlayer(float dt);

		private:

			void	AttachCameraToPlayer(GameWorld* world);

			void	MatchCameraRotation();

			void StopSliding();

			int mMovementSpeed;

			GameWorld* mGameWorld;
		};
	}
}

