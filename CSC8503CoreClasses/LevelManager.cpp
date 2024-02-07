#include "LevelManager.h"
#include "GameWorld.h"
#include <filesystem>

using namespace NCL::CSC8503;

LevelManager::LevelManager() {
	mRoomList = std::vector<Room>();
	for (const auto& entry : std::filesystem::directory_iterator("../Assets/Levels/Rooms")) {
		Room newRoom = Room(entry.path().string());
		mRoomList.push_back(newRoom);
	}
	mLevelList = std::vector<Level>();
	for (const auto& entry : std::filesystem::directory_iterator("../Assets/Levels/Levels")) {
		Level newLevel = Level(entry.path().string());
		mLevelList.push_back(newLevel);
	}
	mActiveLevel = -1;
	mPlayerPosition = Vector3(0, 0, 0);
}

void LevelManager::UpdateLevel() {
}

void LevelManager::LoadLevel(int id, GameWorld* world) {
	if (id > mLevelList.size() - 1 || !world) return;
	LoadMap(mLevelList[id].GetTileMap(), Vector3(0, 0, 0));
	for (auto const& [key, val] : mLevelList[id].GetRooms()) {
		switch (val.GetType()) {
		case Medium:
			for (Room room : mRoomList) {
				if (room.GetType() == Medium) {
					LoadMap(room.GetTileMap(), key);
					break;
				}
			}
			break;
		}
	}
}

float LevelManager::GetSqDistanceToCamera(Vector3& objectPosition) {
	return (objectPosition - mPlayerPosition).LengthSquared();
}

void LevelManager::LoadMap(const std::map<Vector3, TileType>& tileMap, const Vector3& startPosition) {
	for (auto const& [key, val] : tileMap) {
		switch (val) {
		case Wall:
			AddWallToWorld(key + startPosition);
			break;
		case Floor:
			AddFloorToWorld(key + startPosition);
			break;
		}
	}
}

GameObject* LevelManager::AddWallToWorld(const Vector3& position) {
	return nullptr;
}

GameObject* LevelManager::AddFloorToWorld(const Vector3& position) {
	return nullptr;
}
