#include "UI.h"


using namespace NCL;
using namespace CSC8503;

std::vector<UI::Icon> icons;
UI::Icon UI::AddIcon(Vector2 newPos, int horiSize, int vertSize, Texture* tex, bool isShown) {
	Icon icon;
	icon.position = newPos;
	icon.length = horiSize;
	icon.height = vertSize;
	icon.texture = tex;
	icon.isAppear = isShown;
	icons.emplace_back(icon);
	return icon;
}


void UI::SetIconPosition(Vector2 newPos, Icon icon) {
	icon.position = newPos;
}

void UI::SetIconTransparency(bool isShown, Icon icon) {
	icon.isAppear = isShown;
}

const std::vector<UI::Icon>& UI::GetInventorySlot() {
	return icons;
}

void UI::DeleteIcon(Icon icon) {
	if (icons.size() <= 0) {
		return;
	}
	int j = 0;
	for (auto &i : icons) {
		if (i.position == icon.position) {
			icons.erase(icons.begin() + j);
		}
		j++;
	}
}


void UI::BuildVerticesForIcon(const int& iconNum, const Vector2& iconPos, int horiSize, int vertSize, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords) {
	positions.reserve(positions.size() + (iconNum * 6));
	texCoords.reserve(positions.size() + (iconNum * 6));

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