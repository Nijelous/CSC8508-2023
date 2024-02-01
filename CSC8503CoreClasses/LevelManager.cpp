#include "LevelManager.h"

using namespace NCL::CSC8503;

LevelManager::LevelManager() {
	mLevelList = std::vector<Level>();
	mRoomList = std::vector<Room>();
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
