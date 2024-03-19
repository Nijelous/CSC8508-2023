#pragma once
#include "GameObject.h"

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
			void SetNavMeshFlags(int flag);
			virtual bool GetIsOpen() {
				return mIsOpen;
			}
			virtual bool GetIsLock() {
				return mIsLocked;
			}
			
		protected:
			const float initDoorTimer = 10.0f;
			float mTimer;
			bool mIsOpen;
			bool mIsLocked;
		};
	}
}
