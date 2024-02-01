#pragma once
#include "ILevelManager.h"
#include "Level.h"
#include "Room.h"

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class LevelManager : public ILevelManager {
		public:
			LevelManager();
			std::vector<Level> GetLevels() { return mLevelList; }
			std::vector<Room> GetRooms() { return mRoomList; }
			int GetActiveLevel() const { return mActiveLevel; }
			Vector3 GetPlayerPosition() const { return mPlayerPosition; }
			virtual void LoadLevel(int id) override;
			virtual void UpdateLevel() override;
			virtual float GetSqDistanceToCamera(Vector3& objectPosition) override;
		protected:
			std::vector<Level> mLevelList;
			std::vector<Room> mRoomList;
			int mActiveLevel;
			Vector3 mPlayerPosition;
		};
	}
}

