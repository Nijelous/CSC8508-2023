#include "Level.h"
#include "JsonParser.h"
#include <fstream>

using namespace NCL::CSC8503;


Level::Level(std::string levelPath) {
	mLevelName = levelPath.substr(24, levelPath.size()-29);
	mPlayerStartTransforms = new Transform[MAX_PLAYERS];
	std::ifstream levelFile(levelPath);
	std::string line;
	getline(levelFile, line);

	JsonParser parser = JsonParser();

	parser.ParseJson(line, this, nullptr);
}

Level::~Level() {
	for (auto const& [key, val] : mRoomList) {
		delete(val);
		mRoomList.erase(key);
	}
	for (int i = 0; i < mLights.size(); i++) {
		delete(mLights[i]);
	}
	mLights.clear();
	for (int i = 0; i < mVents.size(); i++) {
		delete(mVents[i]);
	}
	mVents.clear();
}
