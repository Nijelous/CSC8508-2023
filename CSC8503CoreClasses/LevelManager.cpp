#include "LevelManager.h"

using namespace NCL::CSC8503;

LevelManager::LevelManager() {
	levelList = std::vector<Level>();
	roomList = std::vector<Room>();
	activeLevel = -1;
	playerPosition = Vector3(0, 0, 0);
}

void LevelManager::UpdateLevel() {
}

void LevelManager::LoadLevel(int id) {
}

float LevelManager::GetDistanceToCamera(Vector3& objectPosition) {
	return (objectPosition - playerPosition).Length();
}
