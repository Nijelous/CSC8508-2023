#include "UI.h"


using namespace NCL;
using namespace CSC8503;

std::vector<UI::Icon> inventorySlot;
void UI::CreateInevntorySlot(Vector2 newPos, int sideLength, Texture* tex) {
	Icon inventory;
	inventory.position = newPos;
	inventory.size = sideLength;
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

void UI::BuildVerticesForIcon(const int& iconNum, const Vector2& iconPos, int size, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords) {
	positions.reserve(positions.size() + (iconNum * 6));
	texCoords.reserve(positions.size() + (iconNum * 6));

	positions.emplace_back(Vector3(iconPos.x, iconPos.y + size, 0));
	positions.emplace_back(Vector3(iconPos.x, iconPos.y, 0));
	positions.emplace_back(Vector3(iconPos.x + size, iconPos.y + size, 0));


	positions.emplace_back(Vector3(iconPos.x, iconPos.y, 0));
	positions.emplace_back(Vector3(iconPos.x + size, iconPos.y + size, 0));
	positions.emplace_back(Vector3(iconPos.x + size, iconPos.y, 0));

	texCoords.emplace_back(Vector2(0, 1));
	texCoords.emplace_back(Vector2(0, 0));
	texCoords.emplace_back(Vector2(1, 1));

	texCoords.emplace_back(Vector2(0, 0));
	texCoords.emplace_back(Vector2(1, 1));
	texCoords.emplace_back(Vector2(1, 0));
}