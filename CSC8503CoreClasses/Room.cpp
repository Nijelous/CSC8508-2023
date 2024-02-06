#include "Room.h"
#include "BaseLight.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"
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

enum RoomVariables {
	SetRoomType,
	TileMap,
	CCTVTransforms,
	Pointlight,
	Spotlight,
	ItemPositions
};

Room::Room(std::string roomPath) {
	mRoomName = roomPath.substr(23, roomPath.size() - 28);
	std::ifstream roomFile(roomPath);
	std::string line;
	getline(roomFile, line);

	std::string output = "";
	std::string currentKey = "";

	int indents = 0;
	int maxIndents = 0;
	bool writingKey = false;
	bool writingValue = false;

	std::vector<std::map<std::string, float>> keyValuePairs = std::vector<std::map<std::string, float>>();
	for (char c : line) {
		switch (c) {
		case '"':
			writingKey = !writingKey;
			currentKey = output;
			WriteValue(!writingKey, &keyValuePairs, output, &output, indents, maxIndents);
			if (writingKey && indents == 1) maxIndents = 1;
			break;
		case '{':
			indents++;
			if (indents > maxIndents) maxIndents++;
			writingValue = false;
			keyValuePairs.push_back(std::map<std::string, float>());
			break;
		case '}':
			WriteValue(writingValue, &keyValuePairs, currentKey, &output, indents, maxIndents);
			writingValue = false;
			indents--;
			break;
		case ',':
			WriteValue(writingValue, &keyValuePairs, currentKey, &output, indents, maxIndents);
			writingValue = false;
			if (indents == 1) {
				WriteVariable(keyValuePairs);
				while (keyValuePairs.size() > 1) keyValuePairs.pop_back();
			}
			break;
		case ':':
			writingValue = true;
			break;
		case '[':
		case ']':
			break;
		default:
			output += c;
			break;
		}
	}
	WriteVariable(keyValuePairs);
}

void Room::WriteValue(bool writingValue, std::vector<std::map<std::string, float>>* keyValuePairs, std::string key, std::string* value, int indents, int maxIndents) {
	if (writingValue) {
		if (keyValuePairs->size() <= maxIndents || indents < maxIndents) {
			(*keyValuePairs)[indents - 1][key] = std::atof(value->c_str());
		}
		else {
			(*keyValuePairs)[keyValuePairs->size() - 1][key] = std::atof(value->c_str());
		}
		*value = "";
	}
}

void Room::WriteVariable(std::vector<std::map<std::string, float>>& keyValuePairs) {
	switch (keyValuePairs[0].size() - 1) {
	case SetRoomType:
		mType = (RoomType)keyValuePairs[0]["type"];
		break;
	case TileMap:
		mTileMap[Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"])] = (TileType)keyValuePairs[1]["type"];
		break;
	case CCTVTransforms:
		mCCTVTransforms.push_back(Matrix4::Rotation(keyValuePairs[3]["y"], Vector3(0, 1, 0)) *
			Matrix4::Translation(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"])) * Matrix4::Scale(Vector3(1, 1, 1)) * Matrix4());
		break;
	case Pointlight:
		mLights.push_back((Light*)new PointLight(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]),
			Vector4(keyValuePairs[3]["x"], keyValuePairs[3]["y"], keyValuePairs[3]["z"], keyValuePairs[3]["w"]), keyValuePairs[1]["radius"]));
		break;
	case Spotlight:
		mLights.push_back((Light*)new SpotLight(Vector3(keyValuePairs[4]["x"], keyValuePairs[4]["y"], -keyValuePairs[4]["z"]),
			Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]),
			Vector4(keyValuePairs[3]["x"], keyValuePairs[3]["y"], keyValuePairs[3]["z"], keyValuePairs[3]["w"]),
			keyValuePairs[1]["radius"], keyValuePairs[1]["angle"], 1.0f));
		break;
	case ItemPositions:
		mItemPositions.push_back(Vector3(keyValuePairs[1]["x"], keyValuePairs[1]["y"], -keyValuePairs[1]["z"]));
		break;
	}
}