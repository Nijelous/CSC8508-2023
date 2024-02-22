#pragma once
#include "Vector2.h"
#include "SimpleFont.h"
#include "../CSC8503/InventoryBuffSystem/PlayerInventory.h"

namespace NCL {

	using namespace Maths;
	using namespace Rendering;

	namespace CSC8503 {
		class UI {
		public:
			UI();
			~UI();

			struct Icon {
				Vector2 position;
				int length;
				int height;
				Texture* texture;
				bool isAppear;
			};
			Icon& AddIcon(Vector2 Pos, int horiSize, int vertSize, Texture* tex, bool isShown = true);
			Icon& AddIcon(Icon* icon, bool isShown = true);

			std::vector<Icon*>& GetIcons();

			void SetIconPosition(Vector2 newPos, Icon icon);

			void SetIconTransparency(bool isShown, Icon icon);

			void DeleteIcon(Icon icon);

			void BuildVerticesForIcon(const Vector2& iconPos, int horiSize, int vertSize, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords);

			void ChangeEquipmentSlotTexture(int slotNum, Texture& texture);

			void SetEquippedItemIcon(int slotNum, Icon& icon);

		protected:
			Icon mIcon;

			Icon* mFirstEquippedItem;
			Icon* mSecondEquippedItem;

			std::vector<UI::Icon*> icons;
		};
	}
}