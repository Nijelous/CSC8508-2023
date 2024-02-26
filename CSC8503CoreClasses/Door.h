#pragma once
#include "GameObject.h"
#include "StateMachine.h"

namespace NCL {
	namespace CSC8503 {
		
		class Door : public GameObject {
		public:
			Door(){
				mName = "Door";
				InitStateMachine();
			}	

			virtual void Open();
			virtual void Close();
			virtual void InitStateMachine();
			virtual void CountDownTimer(float dt);
			
		protected:
			const float initDoorTimer = 10.0f;
			StateMachine* mStateMachine;
			float mTimer;
			bool mIsOpen;
		};
	}
}
