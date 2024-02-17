#include "UI.h"


using namespace NCL;
using namespace CSC8503;

UI::UI() {

}

UI::~UI(){

}


UI::Icon UI::AddIcon(Vector2 Pos, int horiSize, int vertSize, Texture* tex, bool isShown) {
	mIcon.position = Pos;
	mIcon.length = horiSize;
	mIcon.height = vertSize;
	mIcon.texture = tex;
	mIcon.isAppear = isShown;
	icons.emplace_back(mIcon);
	return mIcon;
}


void UI::SetIconPosition(Vector2 newPos, Icon icon) {
	icon.position = newPos;
}

void UI::SetIconTransparency(bool isShown, Icon icon) {
	icon.isAppear = isShown;
}

const std::vector<UI::Icon>& UI::GetIcons() {
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

std::pair<bool, UI::Icon> UI::HaveIcon(Vector2 iconPosition) {
	for (auto& i : icons) {
		if (iconPosition == i.position) {
			return std::make_pair(true, i);
		}
	}
	return std::make_pair(false, mIcon);
}


void UI::BuildVerticesForIcon(const Vector2& iconPos, int horiSize, int vertSize, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords) {
	positions.reserve(positions.size() +  6);
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