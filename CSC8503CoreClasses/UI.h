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
				Texture* texture;
			};
			void CreateInevntorySlot(Vector2 newPos, Texture* tex);

			static const std::vector<Icon>& GetInventorySlot();

			void CreatePowerupIndicator();

			void CreateSuspicionBar();

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