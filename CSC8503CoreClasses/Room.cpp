#include "Room.h"
#include "BaseLight.h"

using namespace NCL::CSC8503;

Room::Room(int type) {
	switch (type) {
	case 0:
		mType = Medium;
		break;
	default:
		mType = INVALID;
		break;
	}
}

Room::Room(std::string roomPath) {
	mType = RoomType::Medium;
	mCCTVPositions = std::vector<Vector3>();
	mLights = std::vector<Light*>();
}
