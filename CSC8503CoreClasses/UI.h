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
			static void CreateInevntorySlot(Vector2 newPos, int horiSize, int vertSize, Texture* tex, bool isShown = true);

			static const std::vector<Icon>& GetInventorySlot();

			static void SetIconPosition(Vector2 newPos, Icon icon);

			static void SwitchTransparency(bool isShown, Icon icon);

			static void BuildVerticesForIcon(const int& iconNum, const Vector2& iconPos, int horiSize, int vertSize, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords);
			/*enum PowerUP{
				SilentRun, SlowDowm, LightOff, IndicateHeist, SwapPosition, Stun, MakeNoise
			};*/
		protected:
			UI();
			~UI();
			Vector2 pos;
			//std::vector<Icon> inventorySlot;
		};
	}
}