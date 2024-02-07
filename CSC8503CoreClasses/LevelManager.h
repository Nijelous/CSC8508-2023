#pragma once
#include "Level.h"

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class GameObject;
		class Mesh;
		class Texture;
		class Shader;
		class LevelManager {
		public:
			LevelManager();
			std::vector<Level> GetLevels() { return mLevelList; }
			std::vector<Room> GetRooms() { return mRoomList; }
			int GetActiveLevel() const { return mActiveLevel; }
			Vector3 GetPlayerPosition() const { return mPlayerPosition; }
			void LoadLevel(int id, GameWorld* world, Mesh* mesh, Texture* floorAlbedo, Texture* floorNormal, Shader* shader);
			float GetSqDistanceToCamera(Vector3& objectPosition);
		protected:
			void LoadMap(const std::map<Vector3, TileType>& tileMap, const Vector3& startPosition, 
				Mesh* mesh, Texture* floorAlbedo, Texture* floorNormal, Shader* shader);

			GameObject* AddWallToWorld(const Vector3& position, Mesh* mesh, Texture* floorAlbedo, Texture* floorNormal, Shader* shader);
			GameObject* AddFloorToWorld(const Vector3& position, Mesh* mesh, Texture* floorAlbedo, Texture* floorNormal, Shader* shader);
			std::vector<Level> mLevelList;
			std::vector<Room> mRoomList;
			int mActiveLevel;
			Vector3 mPlayerPosition;
		};
	}
}

