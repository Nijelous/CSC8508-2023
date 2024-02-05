#include "Level.h"
#include "Vent.h"
#include "GameObject.h"
#include "BaseLight.h"
#include <fstream>

using namespace NCL::CSC8503;

Level::Level(std::string levelPath) {
	std::cout << levelPath << "\n";
	std::ifstream levelFile(levelPath);
	std::string line;
	while (getline(levelFile, line)) {
		std::string output = "";
		int count = 0;
		for (char c : line) {
			if (c == '[' || c == '{') {
				if (!output.empty()) {
					for (int i = 0; i < count; i++) std::cout << "\t";
					std::cout << output << "\n";
				}
				count++;
				output = "";
			}
			else if (c == ']' || c == '}') {
				if (!output.empty()) {
					for (int i = 0; i < count; i++) std::cout << "\t";
					std::cout << output << "\n";
				}
				count--;
				output = "";
				if (c == ']') break;
			}
			else if (c == ',') {
				output += '\n';
				for (int i = 0; i < count; i++) output += '\t';
			}
			else {
				output += c;
			}
		}
	}
	mLevelName = "";
	std::map<Vector3, GameObject*> mTileMap;
	mRoomList = std::map<Vector3, Room>();
	mGuardPaths = std::vector<std::vector<Vector3>>();
	mGuardCount = 0;
	mCCTVTransforms = std::vector<Matrix4>();
	mCCTVCount = 0;
	mPrisonPosition = Vector3(0, 0, 0);
	mPlayerStartPositions = new Vector3[4];
	mLights = std::vector<Light*>();
	//NavMesh
	mItemPositions = std::vector<Vector3>();
	mVents = std::vector<Vent*>();
}
