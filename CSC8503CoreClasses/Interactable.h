#pragma once

namespace NCL {
	namespace CSC8503 {
		class Interactable{
		public:
			virtual void Interact() { 
				if (!CanBeInteractedWith())
					return; 
			};
			virtual bool CanBeInteractedWith() { return mInteractable; };
		protected:
			bool mInteractable = false;
		};
	}
}
