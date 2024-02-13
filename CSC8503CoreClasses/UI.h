#pragma once
#include "Vector2.h"
#include "SimpleFont.h"

namespace NCL {

	using namespace Maths;
	using namespace Rendering;
	namespace CSC8503 {
		class UI {
		public:

			struct Icon {
				Vector2 position;
				int length;
				int height;
				Texture* texture;
				bool isAppear;
			};
			static Icon AddIcon(Vector2 newPos, int horiSize, int vertSize, Texture* tex, bool isShown = true);

			static const std::vector<Icon>& GetInventorySlot();

			static void SetIconPosition(Vector2 newPos, Icon icon);

			static void SetIconTransparency(bool isShown, Icon icon);

			static void DeleteIcon(Icon icon);

			static void BuildVerticesForIcon(const int& iconNum, const Vector2& iconPos, int horiSize, int vertSize, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords);

		protected:
			UI();
			~UI();
			Vector2 mPos;
		};
	}
}