#include "JsonParser.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Vent.h"

using namespace NCL::CSC8503;

constexpr ParserVariables LEVEL_VARIABLES[13] = {
	TileMap,
	RoomList,
	GuardCount,
	CCTVCount,
	GuardPaths,
	CCTVTransforms,
	PrisonPosition,
	PlayerStartPositions,
	DirectionalLight,
	Pointlight,
	Spotlight,
	ItemPositions,
	Vents
};

constexpr ParserVariables ROOM_VARIABLES[6] = {
	SetRoomType,
	TileMap,
	CCTVTransforms,
	Pointlight,
	Spotlight,
	ItemPositions,
};

constexpr int MAX_PLAYERS = 4;

void JsonParser::ParseJson(std::string JSON, Level* level, Room* room) {
	if ((!level && !room) || (level && room)) return;
	std::string output = "";
	std::string currentKey = "";

	int indents = 0;
	int maxIndents = 0;
	bool writingKey = false;
	bool writingValue = false;

	std::vector<std::map<std::string, float>> keyValuePairs = std::vector<std::map<std::string, float>>();
	for (char c : JSON) {
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
				WriteVariable(keyValuePairs, level, room);
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
	WriteVariable(keyValuePairs, level, room);
}

void JsonParser::WriteValue(bool writingValue, std::vector<std::map<std::string, float>>* keyValuePairs, std::string key, std::string* value, int indents, int maxIndents) {
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

void JsonParser::WriteVariable(std::vector<std::map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	ParserVariables variable = level ? LEVEL_VARIABLES[keyValuePairs[0].size() - 1] : ROOM_VARIABLES[keyValuePairs[0].size() - 1];
	switch (variable) {
	case SetRoomType:
		room->mType = (RoomType)keyValuePairs[0]["type"];
		break;

	case TileMap:
	{
		Vector3 key = Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]);
		TileType value = (TileType)keyValuePairs[1]["type"];
		if (level) level->mTileMap[key] = value;
		else room->mTileMap[key] = value;
	}
		break;

	case RoomList:
		level->mRoomList[Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"])] = Room((int)keyValuePairs[1]["type"]);
		break;

	case GuardCount:
		level->mGuardCount = (int)keyValuePairs[0]["guardCount"];
		break;

	case CCTVCount:
		level->mCCTVCount = (int)keyValuePairs[0]["cctvCount"];
		break;

	case GuardPaths:
		level->mGuardPaths.push_back(std::vector<Vector3>());
		for (int i = 2; i < keyValuePairs.size(); i++) {
			level->mGuardPaths[level->mGuardPaths.size() - 1].push_back(Vector3(keyValuePairs[i]["x"], keyValuePairs[i]["y"], -keyValuePairs[i]["z"]));
		}
		break;

	case CCTVTransforms:
	{
		Matrix4 newTransform = Matrix4::Rotation(keyValuePairs[3]["y"], Vector3(0, 1, 0)) *
			Matrix4::Translation(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"])) * Matrix4::Scale(Vector3(1, 1, 1)) * Matrix4();
		if (level) level->mCCTVTransforms.push_back(newTransform);
		else room->mCCTVTransforms.push_back(newTransform);
	}
		break;

	case PrisonPosition:
		level->mPrisonPosition = Vector3(keyValuePairs[1]["x"], keyValuePairs[1]["y"], -keyValuePairs[1]["z"]);
		break;

	case PlayerStartPositions:
		for (int i = 0; i < MAX_PLAYERS; i++) {
			if (!level->mPlayerStartPositions[i].x) {
				level->mPlayerStartPositions[i] = Vector3(keyValuePairs[1]["x"], keyValuePairs[1]["y"], -keyValuePairs[1]["z"]);
				break;
			}
		}
		break;

	case DirectionalLight:
	{
		Light* newLight = (Light*)new DirectionLight(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]),
			Vector4(keyValuePairs[3]["x"], keyValuePairs[3]["y"], keyValuePairs[3]["z"], keyValuePairs[3]["w"]));
		level->mLights.push_back(newLight);
	}
		break;

	case Pointlight:
	{
		Light* newLight = (Light*)new PointLight(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]),
			Vector4(keyValuePairs[3]["x"], keyValuePairs[3]["y"], keyValuePairs[3]["z"], keyValuePairs[3]["w"]), keyValuePairs[1]["radius"]);
			if (level) level->mLights.push_back(newLight);
			else room->mLights.push_back(newLight);
	}
		break;

	case Spotlight:
	{
		Light* newLight = (Light*)new SpotLight(Vector3(keyValuePairs[4]["x"], keyValuePairs[4]["y"], -keyValuePairs[4]["z"]),
			Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]),
			Vector4(keyValuePairs[3]["x"], keyValuePairs[3]["y"], keyValuePairs[3]["z"], keyValuePairs[3]["w"]),
			keyValuePairs[1]["radius"], keyValuePairs[1]["angle"], 1.0f);
		if (level) level->mLights.push_back(newLight);
		else room->mLights.push_back(newLight);
	}
		break;

	case ItemPositions:
	{
		Vector3 newPos = Vector3(keyValuePairs[1]["x"], keyValuePairs[1]["y"], -keyValuePairs[1]["z"]);
		if (level) level->mItemPositions.push_back(newPos);
		else room->mItemPositions.push_back(newPos);
	}
		break;

	case Vents:
		Vent* vent = new Vent();
		vent->GetTransform().SetPosition(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]))
			.SetOrientation(Quaternion::EulerAnglesToQuaternion(keyValuePairs[3]["x"], keyValuePairs[3]["y"], -keyValuePairs[3]["z"]));
		level->mVents.push_back(vent);
		level->mVentConnections.push_back(keyValuePairs[1]["connectedVentID"]);
		break;
	}
}


