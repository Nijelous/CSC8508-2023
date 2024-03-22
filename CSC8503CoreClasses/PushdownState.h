#pragma once

namespace NCL {
	namespace CSC8503 {
		class PushdownState
		{
		public:
			enum PushdownResult {
				Push, Pop, NoChange
			};
			PushdownState()  {
			}
			virtual ~PushdownState() {}

			virtual PushdownResult OnUpdate(float dt, PushdownState** pushFunc) = 0;
			virtual void OnAwake() {}
			virtual void OnSleep() {}
			
		protected:
		};

		class DefaultState : public PushdownState {
			DefaultState() {}
			~DefaultState() {}

			PushdownResult OnUpdate(float dt, PushdownState** pushFunc) {}
			void OnAwake() {}
			void OnSleep() {}
		};
	}
}