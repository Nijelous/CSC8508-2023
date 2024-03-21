#include "UISystem.h"
#include "Vector3.h"

using namespace NCL;
using namespace CSC8503;

namespace {


}

UISystem::UISystem() {

}

UISystem::~UISystem() {

}


UISystem::Icon* UISystem::AddIcon(Vector2 Pos, float horiSize, float vertSize, Texture* tex, float transparency) {
	Icon* mIcon = new Icon();
	mIcon->mPosition = Pos;
	mIcon->mLength = horiSize;
	mIcon->mHeight = vertSize;
	mIcon->mTexture = tex;
	mIcon->mTransparency = transparency;
	mIconsVec.emplace_back(mIcon);
	return mIcon;
}

UISystem::Icon* UISystem::AddIcon(Icon* icon, float transparency) {
	mIconsVec.emplace_back(icon);
	return icon;
}


void UISystem::SetIconPosition(Vector2 newPos, Icon& icon) {
	icon.mPosition = newPos;
}


std::vector<UISystem::Icon*>& UISystem::GetIcons() {
	return mIconsVec;
}

void UISystem::DeleteIcon(Icon icon) {
	if (mIconsVec.size() <= 0) {
		return;
	}
	int j = 0;
	for (auto& i : mIconsVec) {
		if (i->mPosition == icon.mPosition) {
			mIconsVec.erase(mIconsVec.begin() + j);
		}
		j++;
	}
}

void UISystem::BuildVerticesForIcon(const Vector2& iconPos, float horiSize, float vertSize, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords) {
	positions.reserve(positions.size() + 6);
	texCoords.reserve(texCoords.size() + 6);

	positions.emplace_back(Vector3(iconPos.x, iconPos.y + vertSize, 0));
	positions.emplace_back(Vector3(iconPos.x, iconPos.y, 0));
	positions.emplace_back(Vector3(iconPos.x + horiSize, iconPos.y + vertSize, 0));


	positions.emplace_back(Vector3(iconPos.x, iconPos.y, 0));
	positions.emplace_back(Vector3(iconPos.x + horiSize, iconPos.y + vertSize, 0));
	positions.emplace_back(Vector3(iconPos.x + horiSize, iconPos.y, 0));

	texCoords.emplace_back(Vector2(0, 1));
	texCoords.emplace_back(Vector2(0, 0));
	texCoords.emplace_back(Vector2(1, 1));

	texCoords.emplace_back(Vector2(0, 0));
	texCoords.emplace_back(Vector2(1, 1));
	texCoords.emplace_back(Vector2(1, 0));
}


void UISystem::ChangeEquipmentSlotTexture(int slotNum, Texture& texture) {


	switch (slotNum) {
	case FIRST_ITEM_SLOT:
		mFirstEquippedItem->mTexture = &texture;

		break;
	case SECOND_ITEM_SLOT:
		mSecondEquippedItem->mTexture = &texture;
		break;
	default:
		break;
	}
}

void UISystem::ChangeBuffSlotTransparency(int slotNum, float transparency){
	switch (slotNum)
	{
	case FIRST_ITEM_SLOT:
		mFirstEquippedItem->mTransparency = transparency;
		break;
	case SECOND_ITEM_SLOT:
		mSecondEquippedItem->mTransparency = transparency;
		break;
	case SILENT_BUFF_SLOT:
		mSilentSprintIcon->mTransparency = transparency;
		break;
	case SLOW_BUFF_SLOT:
		mSlowIcon->mTransparency = transparency;
		break;
	case STUN_BUFF_SLOT:
		mStunIcon->mTransparency = transparency;
		break;
	case SPEED_BUFF_SLOT:
		mSpeedIcon->mTransparency = transparency;
		break;
	case SUSPISION_BAR_SLOT:
		mSuspensionBarIcon->mTransparency = transparency;
		break;
	case SUSPISION_INDICATOR_SLOT:
		mSuspensionIndicatorIcon->mTransparency = transparency;
		break;
	case CROSS:
		mCross->mTransparency = transparency;
		break;
	case ALARM:
		mAlarm->mTransparency = transparency;
		break;
	case NOTICERIGHT:
		mNoticeRight->mTransparency = transparency;
		break;
	case NOTICELEFT:
		mNoticeLeft->mTransparency = transparency;
		break;
	case NOTICETOP:
		mNoticeTop->mTransparency = transparency;
		break;
	case NOTICEBOT:
		mNoticeBot->mTransparency = transparency;
		break;
	case NOTICEBOTLEFT:
		mNoticeBotLeft->mTransparency = transparency;
		break;
	case NOTICEBOTRIGHT:
		mNoticeBotRight->mTransparency = transparency;
		break;
	case NOTICETOPRIGHT:
		mNoticeTopRight->mTransparency = transparency;
		break;

	default:
		break;
	}

}

void UISystem::SetEquippedItemIcon(int slotNum, Icon& icon) {
	switch (slotNum) {
		case FIRST_ITEM_SLOT:
			mFirstEquippedItem = &icon;
			break;
		case SECOND_ITEM_SLOT:
			mSecondEquippedItem = &icon;
			break;
		case SILENT_BUFF_SLOT:
			mSilentSprintIcon = &icon;
			break;
		case SLOW_BUFF_SLOT:
			mSlowIcon = &icon;
			break;
		case STUN_BUFF_SLOT:
			mStunIcon = &icon;
			break;
		case SPEED_BUFF_SLOT:
			mSpeedIcon = &icon;
			break;
		case SUSPISION_BAR_SLOT:
			mSuspensionBarIcon = &icon;
			break;
		case SUSPISION_INDICATOR_SLOT:
			mSuspensionIndicatorIcon = &icon;
			break;
		case CROSS:
			mCross = &icon;
			break;
		case ALARM:
			mAlarm = &icon;
			break;
		case NOTICERIGHT:
			mNoticeRight = &icon;
			break;
		case NOTICELEFT:
			mNoticeLeft = &icon;
			break;
		case NOTICETOP:
			mNoticeTop = &icon;
			break;
		case NOTICEBOT:
			mNoticeBot = &icon;
			break;
		case NOTICEBOTLEFT:
			mNoticeBotLeft = &icon;
			break;
		case NOTICEBOTRIGHT:
			mNoticeBotRight = &icon;
			break;
		case NOTICETOPRIGHT:
			mNoticeTopRight = &icon;
			break;
	default:
		break;
	}
}
