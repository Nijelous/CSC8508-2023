#pragma once

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class ILevelManager {
		public:
			virtual ~ILevelManager() {}
		protected:
			virtual void UpdateLevel() = 0;
			virtual void LoadLevel(int id) = 0;
			virtual float GetDistanceToCamera(Vector3& objectPosition) = 0; //Array Size = 4
		};
	}
}

