#include "Room.h"

using namespace NCL::CSC8503;

Room::Room() {
	mType = RoomType::Medium;
	mCCTVPositions = std::vector<Vector3>();
}
