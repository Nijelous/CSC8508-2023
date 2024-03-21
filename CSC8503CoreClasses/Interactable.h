#pragma once
#include "../CSC8503/InventoryBuffSystem/PlayerInventory.h"

namespace NCL {
	namespace CSC8503 {
		enum InteractType {
			Use,
			LongUse,
			PickPocket,
			ItemUse
		};

		enum InteractableItems {
			Default,
			InteractableVents,
			InteractableDoors,
			HeistItem
		};

		class GameObject;
		class Interactable{
		public:
			virtual void Interact(InteractType interactType, GameObject* interactingObject = nullptr) { 
				if (!CanBeInteractedWith(interactType))
					return; 
			};
			virtual bool CanBeInteractedWith(InteractType interactType, GameObject* interactingObject = nullptr) { return mInteractable; } ;

			//virtual InventoryBuffSystem::PlayerInventory::item* GetRelatedItem() { return &mRelatedItem; };
		protected:
			bool mInteractable = false;
			InteractableItems mInteractableItemType;
			//InventoryBuffSystem::PlayerInventory::item mRelatedItem;
		};
	}
}
