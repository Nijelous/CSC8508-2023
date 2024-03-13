#pragma once
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		
		class Door {
		public:
			virtual void Open()=0;
			virtual void Close()=0;
			virtual void CountDownTimer(float dt);
			virtual void SetIsOpen(bool isOpen);
			virtual void SetNavMeshFlags(int flag)=0;
		protected:
			const float initDoorTimer = 10.0f;
			float mTimer;
			bool mIsOpen;
		};
	}
}
