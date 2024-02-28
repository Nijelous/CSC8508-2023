#include "UISystem.h"
#include "../CSC8503/InventoryBuffSystem/PlayerInventory.h"


using namespace NCL;
using namespace CSC8503;
using namespace InventoryBuffSystem;

namespace {


}

UISystem::UISystem() {

}

UISystem::~UISystem() {

}


UISystem::Icon* UISystem::AddIcon(Vector2 Pos, int horiSize, int vertSize, Texture* tex, bool isShown) {
	Icon* mIcon = new Icon();
	mIcon->position = Pos;
	mIcon->length = horiSize;
	mIcon->height = vertSize;
	mIcon->texture = tex;
	mIcon->transparency = isShown;
	mIconsVec.emplace_back(mIcon);
	return mIcon;
}

UISystem::Icon* UISystem::AddIcon(Icon* icon, bool isShown) {
	mIconsVec.emplace_back(icon);
	return icon;
}


void UISystem::SetIconPosition(Vector2 newPos, Icon& icon) {
	icon.position = newPos;
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
		if (i->position == icon.position) {
			mIconsVec.erase(mIconsVec.begin() + j);
		}
		j++;
	}
}

void UISystem::BuildVerticesForIcon(const Vector2& iconPos, int horiSize, int vertSize, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords) {
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
		mFirstEquippedItem->texture = &texture;

		break;
	case SECOND_ITEM_SLOT:
		mSecondEquippedItem->texture = &texture;
		break;
	default:
		break;
	}
}

void UISystem::ChangeBuffSlotTransparency(int slotNum, bool isShown){
	switch (slotNum)
	{
	case DISGUISE_BUFF_SLOT:
		mdisguiseBuffIcon->transparency = isShown;
		break;
	case SILENT_BUFF_SLOT:
		mSilentSprintIcon->transparency = isShown;
		break;
	case SLOW_BUFF_SLOT:
		mSlowIcon->transparency = isShown;
		break;
	case STUN_BUFF_SLOT:
		mStunIcon->transparency = isShown;
		break;
	case SPEED_BUFF_SLOT:
		mSpeedIcon->transparency = isShown;
		break;
	case FLAGSIGHT_BUFF_SLOT:
		mFlagSightIcon->transparency = isShown;
		break;
	case SUSPISION_BAR_SLOT:
		mSuspensionBarIcon->transparency = isShown;
		break;
	case SUSPISION_INDICATOR_SLOT:
		mSuspensionIndicatorIcon->transparency = isShown;
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
		case DISGUISE_BUFF_SLOT:
			mdisguiseBuffIcon = &icon;
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
		case FLAGSIGHT_BUFF_SLOT:
			mFlagSightIcon = &icon;
			break;
		case SUSPISION_BAR_SLOT:
			mSuspensionBarIcon = &icon;
			break;
		case SUSPISION_INDICATOR_SLOT:
			mSuspensionIndicatorIcon = &icon;
			break;

	default:
		break;
	}
}
