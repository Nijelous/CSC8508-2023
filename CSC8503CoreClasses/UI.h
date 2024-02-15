#pragma once
#include "Vector2.h"
#include "SimpleFont.h"

namespace NCL {

	using namespace Maths;
	using namespace Rendering;
	namespace CSC8503 {
		class UI {
		public:
			UI();

			struct Icon {
				Vector2 position;
				int length;
				int height;
				Texture* texture;
				bool isAppear;
			};
			Icon AddIcon(Vector2 newPos, int horiSize, int vertSize, Texture* tex, bool isShown = true);

			const std::vector<Icon>& GetInventorySlot();

			void SetIconPosition(Vector2 newPos, Icon icon);

			void SetIconTransparency(bool isShown, Icon icon);

			void DeleteIcon(Icon icon);

			void BuildVerticesForIcon(const Vector2& iconPos, int horiSize, int vertSize, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords);
	
			std::vector<UI::Icon> icons;
		protected:
			~UI();
			Vector2 mPos;
			Icon icon;
		};
	}
}