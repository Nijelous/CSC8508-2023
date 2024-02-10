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
				int size;
				Texture* texture;
			};
			static void CreateInevntorySlot(Vector2 newPos, int sideLength, Texture* tex);

			static const std::vector<Icon>& GetInventorySlot();

			void CreatePowerupIndicator();

			void CreateSuspicionBar();

			static void BuildVerticesForIcon(const int& iconNum, const Vector2& iconPos, int size, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords);
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