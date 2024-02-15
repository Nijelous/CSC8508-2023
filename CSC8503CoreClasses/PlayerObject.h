#pragma once

#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;


		enum PlayerState {
			Walk,
			Sprint,
			Crouch
		};

		class PlayerObject : public GameObject {
		public:
			PlayerObject(GameWorld* world, const std::string& objName = "", int walkSpeed = 40, int sprintSpeed = 50, int crouchSpeed = 35, Vector3 offset = Vector3(0, 0, 0));
			~PlayerObject();

			virtual void UpdateObject(float dt);

		protected:
			bool mIsCrouched;

			int mMovementSpeed;
			int mWalkSpeed;
			int mSprintSpeed;
			int mCrouchSpeed;

			PlayerState mPlayerState;

			GameWorld* mGameWorld;

			virtual void MovePlayer(float dt);

			void RayCastFromPlayer(GameWorld* world);

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
		private:
		};
	}
}

