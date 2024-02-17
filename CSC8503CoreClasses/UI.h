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
			~UI();

			struct Icon {
				Vector2 position;
				int length;
				int height;
				Texture* texture;
				bool isAppear;
			};
			Icon AddIcon(Vector2 Pos, int horiSize, int vertSize, Texture* tex, bool isShown = true);

			const std::vector<Icon>& GetIcons();

			void SetIconPosition(Vector2 newPos, Icon icon);

			void SetIconTransparency(bool isShown, Icon icon);

			void DeleteIcon(Icon icon);

			std::pair<bool, Icon> HaveIcon(Vector2 iconPosition);

			void BuildVerticesForIcon(const Vector2& iconPos, int horiSize, int vertSize, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords);
	
			
			std::vector<UI::Icon> icons;
		protected:
			Icon mIcon;
		};
	}
}