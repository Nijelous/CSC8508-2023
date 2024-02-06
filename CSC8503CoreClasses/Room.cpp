#include "Room.h"
#include "JsonParser.h"
#include <fstream>

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
	mRoomName = roomPath.substr(23, roomPath.size() - 28);
	std::ifstream roomFile(roomPath);
	std::string line;
	getline(roomFile, line);

	JsonParser parser = JsonParser();

	parser.ParseJson(line, nullptr, this);
}