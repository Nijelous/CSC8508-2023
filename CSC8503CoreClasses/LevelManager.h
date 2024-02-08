#pragma once
#include "Level.h"
#include "../OpenGLRendering/OGLRenderer.h"

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class GameObject;
		class LevelManager {
		public:
			LevelManager();
			~LevelManager();
			std::vector<Level*> GetLevels() { return mLevelList; }
			std::vector<Room*> GetRooms() { return mRoomList; }
			int GetActiveLevel() const { return mActiveLevel; }
			Vector3 GetPlayerPosition() const { return mPlayerPosition; }
			Vector3 GetPlayerStartPosition(int player) const { return (*mLevelList[player]).GetPlayerStartPosition(player)*10; }
			void LoadLevel(int id, GameWorld* world, Mesh* mesh, Texture* albedo, Texture* normal, Shader* shader);
			float GetSqDistanceToCamera(Vector3& objectPosition);
		protected:
			void LoadMap(GameWorld* world, const std::map<Vector3, TileType>& tileMap, const Vector3& startPosition, 
				Mesh* mesh, Texture* albedo, Texture* normal, Shader* shader);

			GameObject* AddWallToWorld(GameWorld* world, const Vector3& position, Mesh* mesh, Texture* albedo, Texture* normal, Shader* shader);
			GameObject* AddFloorToWorld(GameWorld* world, const Vector3& position, Mesh* mesh, Texture* albedo, Texture* normal, Shader* shader);
			std::vector<Level*> mLevelList;
			std::vector<Room*> mRoomList;
			int mActiveLevel;
			Vector3 mPlayerPosition;
		};
	}
}

