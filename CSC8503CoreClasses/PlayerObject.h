#pragma once

#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class PlayerObject : public GameObject {
		public:
			PlayerObject(const std::string& objName = "", int movementSpeed = 500);
			~PlayerObject();

			void	UpdateObject(float dt);

		protected:


		private:

			void	MovePlayer(float dt);

			Vector3	GetForwardAxis();

			Vector3	GetRightAxis();

			int mMovementSpeed;
		};
	}
}

