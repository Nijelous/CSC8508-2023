#include "Room.h"
#include "JsonParser.h"
#include <fstream>

using namespace NCL::CSC8503;

Room::Room(int type, int doorPos, int primaryDoor) {
	switch (type) {
	case 0:
		mType = Small;
		break;
	case 1:
		mType = Medium;
		break;
	case 2:
		mType = Large;
		break;
	case 3:
		mType = LShape;
		break;
	default:
		mType = INVALID;
		break;
	}
	mDoorConfig = doorPos;
	mPrimaryDoor = primaryDoor;
}

Room::Room(std::string roomPath) {
	mRoomName = roomPath.substr(23, roomPath.size() - 28);
	std::ifstream roomFile(roomPath);
	std::string line;
	getline(roomFile, line);

	JsonParser parser = JsonParser();

	parser.ParseJson(line, nullptr, this);
}

Room::~Room() {
	for (int i = 0; i < mLights.size(); i++) {
		delete(mLights[i]);
		mLights[i] = NULL;
	}
}
