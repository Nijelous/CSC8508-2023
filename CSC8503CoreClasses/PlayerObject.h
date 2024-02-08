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
			PlayerObject(GameWorld* world, const std::string& objName = "", int walkSpeed = 35, int sprintSpeed = 50, int crouchSpeed = 25, Vector3 offset = Vector3(0,0,0));
			~PlayerObject();

			void	UpdateObject(float dt);

		protected:
			bool mIsCrouched;
			
			int mMovementSpeed;
			int mWalkSpeed;
			int mSprintSpeed;
			int mCrouchSpeed;

			PlayerState mPlayerState;

			GameWorld* mGameWorld;
			
            virtual void MovePlayer(float dt);
            
			void AttachCameraToPlayer(GameWorld* world);
			
			void MatchCameraRotation();

			void StopSliding();
			
			void	ToggleCrouch();

			void	ActivateSprint();	

			void	StartWalking();

			void	StartSprinting();

			void	StartCrouching();
		private:
		};
	}
}

