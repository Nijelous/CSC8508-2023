#include "Room.h"
#include "BaseLight.h"

using namespace NCL::CSC8503;

Room::Room(std::string roomPath) {
	std::cout << roomPath << "\n";
	mType = RoomType::Medium;
	mCCTVPositions = std::vector<Vector3>();
	mLights = std::vector<Light*>();
}
