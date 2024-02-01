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
			std::vector<Level> GetLevels() { return levelList; }
			std::vector<Room> GetRooms() { return roomList; }
			int GetActiveLevel() const { return activeLevel; }
			Vector3 GetPlayerPosition() const { return playerPosition; }
		protected:
			std::vector<Level> levelList;
			std::vector<Room> roomList;
			int activeLevel;
			Vector3 playerPosition;

			virtual void UpdateLevel();
			virtual void LoadLevel(int id);
			virtual float GetDistanceToCamera(Vector3& objectPosition);

		};
	}
}

