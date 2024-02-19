#pragma once

namespace NCL {
	namespace CSC8503 {

		enum InteractType {
			Use,
			ItemUse
		};

		class Interactable{
		public:
			virtual void Interact(InteractType interactType) { 
				if (!CanBeInteractedWith())
					return; 
			};
			virtual bool CanBeInteractedWith() { return mInteractable; };
		protected:
			bool mInteractable = false;
		};
	}
}
