#include "Level.h"
#include "Vent.h"
#include "GameObject.h"
#include "BaseLight.h"
#include <fstream>

using namespace NCL::CSC8503;

Level::Level(std::string levelPath) {
	mLevelName = levelPath;
	std::ifstream levelFile(levelPath);
	std::string line;
	while (getline(levelFile, line)) {
		std::string output = "";
		std::string currentKey = "";
		int indents = 0;
		bool exit = false;
		bool writingKey = false;
		bool writingValue = false;
		std::vector<std::map<std::string, float>> keyValuePairs = std::vector<std::map<std::string, float>>();
		for (char c : line) {
			switch (c) {
			case '"':
				writingKey = !writingKey;
				if (!writingKey) {
					keyValuePairs[keyValuePairs.size()-1][output] = 0;
					currentKey = output;
					output = "";
				}
				break;
			case '{':
				indents++;
				writingValue = false;
				keyValuePairs.push_back(std::map<std::string, float>());
				break;
			case '[':
				if (keyValuePairs[0].size() == 6) exit = true;
				break;
			case '}':
				indents--;
				if (writingValue) {
					keyValuePairs[keyValuePairs.size() - 1][currentKey] = std::stof(output);
					writingValue = false;
					output = "";
				}
				break;
			case ',':
				if (writingValue) {
					keyValuePairs[keyValuePairs.size()-1][currentKey] = std::stof(output);
					writingValue = false;
					output = "";
				}
				if (indents == 1) {
					WriteValue(keyValuePairs);
					while (keyValuePairs.size() > 1) {
						keyValuePairs.pop_back();
					}
				}
				break;
			case ']':
				break;
			case ':':
				writingValue = true;
				break;
			default:
				output += c;
				break;
			}
			if (exit) break;
		}
	}
	std::cout << "Tile Map" << "\n";
	for (auto const& [key, val] : mTileMap) {
		std::cout << key << ": " << val << "\n";
	}
	std::cout << "Rooms" << "\n";
	for (auto const& [key, val] : mRoomList) {
		std::cout << key << ": " << val.GetType() << "\n";
	}
	std::cout << "Guard Count: " << mGuardCount << "\n";
	std::cout << "CCTV Count: " << mCCTVCount << "\n";
	std::cout << "Guard Paths\n";
	for (int i = 0; i < mGuardPaths.size(); i++) {
		for (int j = 0; j < mGuardPaths[i].size(); j++) {
			std::cout << mGuardPaths[i][j] << ", ";
		}
		std::cout << "\n";
	}
	mGuardPaths = std::vector<std::vector<Vector3>>();
	mCCTVTransforms = std::vector<Matrix4>();
	mPlayerStartPositions = new Vector3[4];
	mLights = std::vector<Light*>();
	//NavMesh
	mItemPositions = std::vector<Vector3>();
	mVents = std::vector<Vent*>();
}

void Level::WriteValue(std::vector<std::map<std::string, float>>& keyValuePairs) {
	switch (keyValuePairs[0].size()) {
	case 1:
		mTileMap[Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"])] = new GameObject();
		break;
	case 2:
		mRoomList[Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"])] = Room((int)keyValuePairs[1]["type"]);
		break;
	case 3:
		mGuardCount = (int)keyValuePairs[0]["guardCount"];
		break;
	case 4:
		mCCTVCount = (int)keyValuePairs[0]["cctvCount"];
		break;
	case 5:
		mGuardPaths.push_back(std::vector<Vector3>());
		for (int i = 2; i < keyValuePairs.size(); i++) {
			mGuardPaths[mGuardPaths.size()-1].push_back(Vector3(keyValuePairs[i]["x"], keyValuePairs[i]["y"], -keyValuePairs[i]["z"]));
		}
		break;
	}
}
