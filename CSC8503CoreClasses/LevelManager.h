#pragma once
#include "Level.h"
#include "Room.h"

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class GameObject;
		class LevelManager {
		public:
			LevelManager();
			std::vector<Level> GetLevels() { return mLevelList; }
			std::vector<Room> GetRooms() { return mRoomList; }
			int GetActiveLevel() const { return mActiveLevel; }
			Vector3 GetPlayerPosition() const { return mPlayerPosition; }
			void LoadLevel(int id, GameWorld* world);
			void UpdateLevel();
			float GetSqDistanceToCamera(Vector3& objectPosition);
		protected:
			void LoadMap(const std::map<Vector3, TileType>& tileMap, const Vector3& startPosition);

			GameObject* AddWallToWorld(const Vector3& position);
			GameObject* AddFloorToWorld(const Vector3& position);
			std::vector<Level> mLevelList;
			std::vector<Room> mRoomList;
			int mActiveLevel;
			Vector3 mPlayerPosition;
		};
	}
}

