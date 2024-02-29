#include "JsonParser.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Door.h"
#include "PrisonDoor.h"
#include "Vent.h"

using namespace NCL::CSC8503;

constexpr ParserVariables LEVEL_VARIABLES[16] = {
	TileMap,
	RoomList,
	GuardCount,
	CCTVCount,
	GuardPaths,
	CCTVTransforms,
	PrisonPosition,
	PlayerStartTransforms,
	DirectionalLight,
	Pointlight,
	Spotlight,
	ItemPositions,
	Vents,
	Helipad,
	PrisonDoorPos,
	Doors
};

constexpr ParserVariables ROOM_VARIABLES[7] = {
	SetRoomType,
	TileMap,
	CCTVTransforms,
	Pointlight,
	Spotlight,
	ItemPositions,
	Doors
};

void JsonParser::ParseJson(std::string JSON, Level* level, Room* room) {
	if ((!level && !room) || (level && room)) return;
	mPlayerCount = 0;
	std::string output = "";
	std::string currentKey = "";

	int indents = 0;
	int maxIndents = 0;
	bool writingKey = false;
	bool writingValue = false;

	std::vector<std::unordered_map<std::string, float>> keyValuePairs = std::vector<std::unordered_map<std::string, float>>();
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
			keyValuePairs.push_back(std::unordered_map<std::string, float>());
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

void JsonParser::WriteValue(bool writingValue, std::vector<std::unordered_map<std::string, float>>* keyValuePairs,
	std::string key, std::string* value, int indents, int maxIndents) {
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

void JsonParser::WriteVariable(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	ParserVariables variable = level ? LEVEL_VARIABLES[keyValuePairs[0].size() - 1] : ROOM_VARIABLES[keyValuePairs[0].size() - 1];
	switch (variable) {
	case SetRoomType:
		room->mType = (RoomType)keyValuePairs[0]["type"];
		break;

	case TileMap:
	{
		Transform key = Transform();
		key.SetPosition(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]))
			.SetOrientation(Quaternion(keyValuePairs[3]["x"], keyValuePairs[3]["w"], keyValuePairs[3]["z"], keyValuePairs[3]["y"]));
		TileType value = (TileType)keyValuePairs[1]["type"];
		if (level) level->mTileMap[key] = value;
		else room->mTileMap[key] = value;
	}
		break;

	case RoomList:
		if (keyValuePairs.size() == 1) return;
		level->mRoomList[Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"])] = new Room((int)keyValuePairs[1]["type"]);
		break;

	case GuardCount:
		level->mGuardCount = (int)keyValuePairs[0]["guardCount"];
		break;

	case CCTVCount:
		level->mCCTVCount = (int)keyValuePairs[0]["cctvCount"];
		break;

	case GuardPaths:
		if (keyValuePairs.size() == 1) return;
		level->mGuardPaths.push_back(std::vector<Vector3>());
		for (int i = 2; i < keyValuePairs.size(); i++) {
			level->mGuardPaths[level->mGuardPaths.size() - 1].push_back(Vector3(keyValuePairs[i]["x"], keyValuePairs[i]["y"], -keyValuePairs[i]["z"]));
		}
		break;

	case CCTVTransforms:
		if (keyValuePairs.size() == 1) return;
	{
		Transform newTransform = Transform();
		newTransform.SetPosition(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]))
			.SetOrientation(Quaternion(keyValuePairs[3]["x"], keyValuePairs[3]["w"], keyValuePairs[3]["z"], keyValuePairs[3]["y"]));
		if (level) level->mCCTVTransforms.push_back(newTransform);
		else room->mCCTVTransforms.push_back(newTransform);
	}
		break;

	case PrisonPosition:
		level->mPrisonPosition = Vector3(keyValuePairs[1]["x"], keyValuePairs[1]["y"], -keyValuePairs[1]["z"]);
		break;

	case PlayerStartTransforms:
		if (mPlayerCount < MAX_PLAYERS) {
			Transform newTransform = Transform();
			newTransform.SetPosition(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]))
				.SetOrientation(Quaternion::EulerAnglesToQuaternion(keyValuePairs[3]["x"], keyValuePairs[3]["y"]-180, keyValuePairs[3]["z"]));
			level->mPlayerStartTransforms[mPlayerCount] = newTransform;
			mPlayerCount++;
		}
		break;

	/*case DirectionalLight:
	{
		Light* newLight = (Light*)new DirectionLight(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]),
			Vector4(keyValuePairs[3]["x"], keyValuePairs[3]["y"], keyValuePairs[3]["z"], keyValuePairs[3]["w"]));
		level->mLights.push_back(newLight);
	}
		break;*/

	case Pointlight:
		if (keyValuePairs.size() == 1) return;
	{
		Light* newLight = (Light*)new PointLight(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]),
			Vector4(keyValuePairs[3]["x"], keyValuePairs[3]["y"], keyValuePairs[3]["z"], keyValuePairs[3]["w"]), keyValuePairs[1]["radius"]);
			if (level) level->mLights.push_back(newLight);
			else room->mLights.push_back(newLight);
	}
		break;

	case Spotlight:
		if (keyValuePairs.size() == 1) return;
	{
			Matrix4 xRot = Matrix4::Rotation(keyValuePairs[4]["x"], Vector3(-1, 0, 0));
			Matrix4 yRot = Matrix4::Rotation(keyValuePairs[4]["y"]-180, Vector3(0, 1, 0));
			Matrix4 zRot = Matrix4::Rotation(-keyValuePairs[4]["z"], Vector3(0, 0, 1));
			Vector3 direction = xRot * yRot * zRot * Vector3(0, 0, 1);
			Light* newLight = (Light*)new SpotLight(direction,
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
		if (keyValuePairs.size() == 1) return;
	{
		Vent* vent = new Vent();
		vent->GetTransform().SetPosition(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]))
			.SetOrientation(Quaternion::EulerAnglesToQuaternion(keyValuePairs[3]["x"], keyValuePairs[3]["y"]-180, -keyValuePairs[3]["z"]));
		level->mVents.push_back(vent);
		level->mVentConnections.push_back(keyValuePairs[1]["connectedVentID"]);
	}
		break;
	case Helipad:
		level->mHelipadPosition = Vector3(keyValuePairs[1]["x"], keyValuePairs[1]["y"], -keyValuePairs[1]["z"]);
		break;
	case Doors:
		if (keyValuePairs.size() == 1) return;
		{
			Door* door = new Door();
			door->GetTransform().SetPosition(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]))
				.SetOrientation(Quaternion::EulerAnglesToQuaternion(keyValuePairs[3]["x"], keyValuePairs[3]["y"] - 180, -keyValuePairs[3]["z"]));
			if (level) level->mDoors.push_back(door);
			else room->mDoors.push_back(door);
		}
		break;
	case PrisonDoorPos:
	{
		PrisonDoor* pDoor = new PrisonDoor();
		pDoor->GetTransform().SetPosition(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]))
			.SetOrientation(Quaternion::EulerAnglesToQuaternion(keyValuePairs[3]["x"], keyValuePairs[3]["y"] - 180, -keyValuePairs[3]["z"]));
		level->mPrisonDoor = pDoor;
	}
	break;
	}
}


