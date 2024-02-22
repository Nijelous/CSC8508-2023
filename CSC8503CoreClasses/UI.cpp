#include "UI.h"
#include "../CSC8503/InventoryBuffSystem/PlayerInventory.h"


using namespace NCL;
using namespace CSC8503;
using namespace InventoryBuffSystem;

namespace {
	constexpr  int FIRST_ITEM_SLOT = 0;
	constexpr int SECOND_ITEM_SLOT = 1;
}

UI::UI() {
}

UI::~UI() {

}


UI::Icon& UI::AddIcon(Vector2 Pos, int horiSize, int vertSize, Texture* tex, bool isShown) {
	Icon* icon = new Icon();
	icon->position = Pos;
	icon->length = horiSize;
	icon->height = vertSize;
	icon->texture = tex;
	icon->isAppear = isShown;
	icons.emplace_back(icon);
	return *icon;
}

UI::Icon& NCL::CSC8503::UI::AddIcon(Icon* icon, bool isShown) {
	icons.emplace_back(icon);
	return *icon;
}


void UI::SetIconPosition(Vector2 newPos, Icon icon) {
	icon.position = newPos;
}

void UI::SetIconTransparency(bool isShown, Icon icon) {
	icon.isAppear = isShown;
}

std::vector<UI::Icon*>& UI::GetIcons() {
	return icons;
}

void UI::DeleteIcon(Icon icon) {
	if (icons.size() <= 0) {
		return;
	}
	int j = 0;
	for (auto& i : icons) {
		if (i->position == icon.position) {
			icons.erase(icons.begin() + j);
		}
		j++;
	}
}


void UI::BuildVerticesForIcon(const Vector2& iconPos, int horiSize, int vertSize, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords) {
	positions.reserve(positions.size() + 6);
	texCoords.reserve(positions.size() + 6);

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

void NCL::CSC8503::UI::ChangeEquipmentSlotTexture(int slotNum, Texture& texture) {
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

void NCL::CSC8503::UI::SetEquippedItemIcon(int slotNum, Icon& icon) {
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
