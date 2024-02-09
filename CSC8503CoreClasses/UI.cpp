#include "UI.h"


using namespace NCL;
using namespace CSC8503;

std::vector<UI::Icon> inventorySlot;
void UI::CreateInevntorySlot(Vector2 newPos, Texture* tex) {
	Icon inventory;
	inventory.position = newPos;
	inventory.texture = tex;
	inventorySlot.emplace_back(inventory);
}

void UI::CreatePowerupIndicator() {
	Vector2 slienceRunIcon;
	std::vector<Vector2> powerupIcons;
	powerupIcons.resize(8);
	for (int i = 0; i < 8; i++) {
		if (i < 4) {
			powerupIcons.emplace_back(Vector2(i + 2, 80));
		}
		else {
			powerupIcons.emplace_back(Vector2(i + 2, 82));
		}
	}
}

void UI::CreateSuspicionBar() {
	Vector2 position;

}

const std::vector<UI::Icon>& UI::GetInventorySlot() {
	return inventorySlot;
}