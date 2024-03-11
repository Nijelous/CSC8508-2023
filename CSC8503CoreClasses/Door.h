#pragma once
#include "GameObject.h"
#include "StateMachine.h"

namespace NCL {
	namespace CSC8503 {
		
		class Door : public GameObject {
		public:
			Door(){
				mName = "Door";
			}	

			virtual void Open();
			virtual void Close();
			virtual void CountDownTimer(float dt);
			virtual void UpdateObject(float dt) override;
			virtual void SetIsOpen(bool isOpen);
		protected:
			void SetNavMeshFlags(int flag);

			const float initDoorTimer = 10.0f;
			float mTimer;
			bool mIsOpen;
		};
	}
}
