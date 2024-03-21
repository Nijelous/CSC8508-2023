#pragma once
#include "Vector2.h"
#include "SimpleFont.h"

namespace NCL {

	using namespace Maths;
	using namespace Rendering;
	using namespace std;

	namespace CSC8503 {
		constexpr int FIRST_ITEM_SLOT = 0;
		constexpr int SECOND_ITEM_SLOT = 1;
		constexpr int SILENT_BUFF_SLOT = 2;
		constexpr int SLOW_BUFF_SLOT = 3;
		constexpr int STUN_BUFF_SLOT = 4;
		constexpr int SPEED_BUFF_SLOT = 5;
		constexpr int SUSPISION_BAR_SLOT = 6;
		constexpr int SUSPISION_INDICATOR_SLOT = 7;
		constexpr int CROSS = 8;
		constexpr int ALARM = 9;
		constexpr int NOTICERIGHT = 10;
		constexpr int NOTICELEFT = 11;
		constexpr int NOTICETOP = 12;
		constexpr int NOTICEBOT = 13;
		constexpr int NOTICEBOTLEFT = 14;
		constexpr int NOTICEBOTRIGHT = 15;
		constexpr int NOTICETOPRIGHT = 16;


		class UISystem {
		public:
			UISystem();
			~UISystem();

			struct Icon {
				Vector2 mPosition;
				float mLength;
				float mHeight;
				Texture* mTexture;
				float mTransparency;
				void SetIconSize(float length, float height) {
					mLength = length;
					mHeight = height;
					
				}
			};
			Icon* AddIcon(Vector2 Pos, float horiSize, float vertSize, Texture* tex, float transparency=1.0);
			Icon* AddIcon(Icon* icon, float transparency=1.0);

			std::vector<Icon*>& GetIcons();

			void SetIconPosition(Vector2 newPos, Icon& icon);

			void DeleteIcon(Icon icon);

			void BuildVerticesForIcon(const Vector2& iconPos, float horiSize, float vertSize, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords);

			void ChangeEquipmentSlotTexture(int slotNum, Texture& texture);

			void ChangeBuffSlotTransparency(int slotNum, float transparency);

			void SetEquippedItemIcon(int slotNum, Icon& icon);

			void SetTextureVector(string itemType, std::vector<Texture*> itemTexVec) {
				if (itemType == "key") {
					mKeyTexVec = itemTexVec;
				}
				if (itemType == "bar") {
					mSusBarTexVec = itemTexVec;
				}
			}

			vector<Texture*> GetKeyTexVec(){
				return mKeyTexVec;
			}

			vector<Texture*> GetSusBarTexVec() {
				return mSusBarTexVec;
			}




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
			Icon* mCross;
			Icon* mAlarm;
			Icon* mNoticeRight;
			Icon* mNoticeLeft;
			Icon* mNoticeTop;
			Icon* mNoticeBot;
			Icon* mNoticeBotLeft;
			Icon* mNoticeBotRight;
			Icon* mNoticeTopRight;
			std::vector<Icon*> mIconsVec;
			std::vector<Texture*> mKeyTexVec;
			std::vector<Texture*> mSusBarTexVec;


		};
	}
}