#pragma once
#include "../CSC8503/InventoryBuffSystem/PlayerInventory.h"

namespace NCL {
	namespace CSC8503 {
		enum InteractType
		{
			Use,
			LongUse, 
			Item
		};

		class Interactable{
		public:
			virtual void Interact(InteractType interactType) { 
				if (!CanBeInteractedWith(interactType))
					return; 
			};
			virtual bool CanBeInteractedWith(InteractType interactType) { return mInteractable; } ;

			virtual InventoryBuffSystem::PlayerInventory::item GetRelatedItem() { return mRelatedItem; };
		protected:
			bool mInteractable = false;
			InventoryBuffSystem::PlayerInventory::item mRelatedItem;
		};
	}
}
