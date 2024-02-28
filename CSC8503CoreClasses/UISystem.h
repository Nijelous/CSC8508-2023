#pragma once
#include "Vector2.h"
#include "SimpleFont.h"
#include "../CSC8503/InventoryBuffSystem/PlayerInventory.h"

namespace NCL {

	using namespace Maths;
	using namespace Rendering;

	namespace CSC8503 {
		constexpr int FIRST_ITEM_SLOT = 0;
		constexpr int SECOND_ITEM_SLOT = 1;
		constexpr int DISGUISE_BUFF_SLOT = 2;
		constexpr int SILENT_BUFF_SLOT = 3;
		constexpr int SLOW_BUFF_SLOT = 4;
		constexpr int STUN_BUFF_SLOT = 5;
		constexpr int SPEED_BUFF_SLOT = 6;
		constexpr int FLAGSIGHT_BUFF_SLOT = 7;
		constexpr int SUSPISION_BAR_SLOT = 8;
		constexpr int SUSPISION_INDICATOR_SLOT = 9;


		class UISystem {
		public:
			UISystem();
			~UISystem();

			struct Icon {
				Vector2 position;
				int length;
				int height;
				Texture* texture;
				bool transparency;
			};
			Icon* AddIcon(Vector2 Pos, int horiSize, int vertSize, Texture* tex, bool isShown = true);
			Icon* AddIcon(Icon* icon, bool isShown = true);

			std::vector<Icon*>& GetIcons();

			void SetIconPosition(Vector2 newPos, Icon& icon);

			void DeleteIcon(Icon icon);

			void BuildVerticesForIcon(const Vector2& iconPos, int horiSize, int vertSize, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords);

			void ChangeEquipmentSlotTexture(int slotNum, Texture& texture);

			void ChangeBuffSlotTransparency(int slotNum, bool isShown);

			void SetEquippedItemIcon(int slotNum, Icon& icon);




		protected:
			Icon mIcon;

			Icon* mFirstEquippedItem;
			Icon* mSecondEquippedItem;

			Icon* mdisguiseBuffIcon;
			Icon* mSilentSprintIcon;
			Icon* mSlowIcon;
			Icon* mStunIcon;
			Icon* mSpeedIcon;
			Icon* mFlagSightIcon;

			Icon* mSuspensionBarIcon;
			Icon* mSuspensionIndicatorIcon;

			std::vector<Icon*> mIconsVec;
		};
	}
}