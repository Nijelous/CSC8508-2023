#include "Level.h"
#include "Vent.h"
#include "JsonParser.h"
#include <fstream>

using namespace NCL::CSC8503;

constexpr int MAX_PLAYERS = 4;

Level::Level(std::string levelPath) {
	mLevelName = levelPath.substr(24, levelPath.size()-29);
	mPlayerStartPositions = new Vector3[MAX_PLAYERS];
	std::ifstream levelFile(levelPath);
	std::string line;
	getline(levelFile, line);

	JsonParser parser = JsonParser();

	parser.ParseJson(line, this, nullptr);

	for (int i = 0; i < mVentConnections.size(); i++) {
		mVents[i]->ConnectVent(mVents[mVentConnections[i]]);
	}
}

Level::~Level() {
	for (int i = 0; i < mLights.size(); i++) {
		delete(mLights[i]);
		mLights[i] = NULL;
	}
	for (int i = 0; i < mVents.size(); i++) {
		delete(mVents[i]);
		mVents[i] = NULL;
	}
}
