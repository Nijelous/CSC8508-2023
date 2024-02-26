#include "UISystem.h"
#include "../CSC8503/InventoryBuffSystem/PlayerInventory.h"


using namespace NCL;
using namespace CSC8503;
using namespace InventoryBuffSystem;

namespace {
	constexpr  int FIRST_ITEM_SLOT = 0;
	constexpr int SECOND_ITEM_SLOT = 1;
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
	mIcon->isAppear = isShown;
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

void UISystem::SetIconTransparency(bool isShown, Icon& icon) {
	icon.isAppear = isShown;
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

void UISystem::SetEquippedItemIcon(int slotNum, Icon& icon) {
	switch (slotNum) {
		case FIRST_ITEM_SLOT:
			mFirstEquippedItem = &icon;
		break;
		case SECOND_ITEM_SLOT:
			mSecondEquippedItem = &icon;
			break;
	default:
		break;
	}
}
