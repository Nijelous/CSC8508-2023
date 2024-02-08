#include "LevelManager.h"
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

void LevelManager::LoadLevel(int id) {
}

float LevelManager::GetSqDistanceToCamera(Vector3& objectPosition) {
	return (objectPosition - mPlayerPosition).LengthSquared();
}
