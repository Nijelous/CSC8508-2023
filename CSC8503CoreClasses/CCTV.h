#pragma once
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class CCTV : public GameObject {
		public:
			CCTV(const std::string& name = "");
			~CCTV() {};

			void UpdateObject(float dt) override;
		protected:

		};
	}
}

