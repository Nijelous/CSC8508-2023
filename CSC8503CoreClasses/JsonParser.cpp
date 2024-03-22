#include "JsonParser.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Door.h"
#include "PrisonDoor.h"
#include "Vent.h"
#include "InteractableDoor.h"

using namespace NCL::CSC8503;

constexpr ParserVariables LEVEL_VARIABLES[17] = {
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
	Doors,
	DecorationTransforms
};

constexpr ParserVariables ROOM_VARIABLES[9] = {
	SetRoomType,
	RoomDoorPos,
	TileMap,
	CCTVTransforms,
	Pointlight,
	Spotlight,
	ItemPositions,
	Doors,
	DecorationTransforms
};

JsonParser::JsonParser() {
	mVariableWriteMap[SetRoomType] = std::bind(&JsonParser::WriteRoomType, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[RoomDoorPos] = std::bind(&JsonParser::WriteRoomDoorPos, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[TileMap] = std::bind(&JsonParser::WriteTileMap, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[RoomList] = std::bind(&JsonParser::WriteRoomList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[GuardCount] = std::bind(&JsonParser::WriteGuardCount, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[CCTVCount] = std::bind(&JsonParser::WriteCCTVCount, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[GuardPaths] = std::bind(&JsonParser::WriteGuardPaths, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[CCTVTransforms] = std::bind(&JsonParser::WriteCCTVTransforms, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[PrisonPosition] = std::bind(&JsonParser::WritePrisonPosition, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[PlayerStartTransforms] = std::bind(&JsonParser::WritePlayerStartTransforms, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[DirectionalLight] = std::bind(&JsonParser::WriteDirectionalLight, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[Pointlight] = std::bind(&JsonParser::WritePointlight, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[Spotlight] = std::bind(&JsonParser::WriteSpotlight, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[ItemPositions] = std::bind(&JsonParser::WriteItemPositions, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[Vents] = std::bind(&JsonParser::WriteVents, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[Helipad] = std::bind(&JsonParser::WriteHelipad, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[Doors] = std::bind(&JsonParser::WriteDoors, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[PrisonDoorPos] = std::bind(&JsonParser::WritePrisonDoorPos, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	mVariableWriteMap[DecorationTransforms] = std::bind(&JsonParser::WriteDecorationTransforms, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

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
				mVariableWriteMap[level ? LEVEL_VARIABLES[keyValuePairs[0].size() - 1] : ROOM_VARIABLES[keyValuePairs[0].size() - 1]](keyValuePairs, level, room);
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
	mVariableWriteMap[level ? LEVEL_VARIABLES[keyValuePairs[0].size() - 1] : ROOM_VARIABLES[keyValuePairs[0].size() - 1]](keyValuePairs, level, room);
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
void JsonParser::WriteRoomType(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	room->mType = (RoomType)keyValuePairs[0]["type"];
}

void JsonParser::WriteRoomDoorPos(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	room->mDoorConfig = keyValuePairs[0]["doorPositions"];
	room->mPrimaryDoor = 0;
}

void JsonParser::WriteTileMap(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	Transform key = Transform();
	key.SetPosition(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]))
		.SetOrientation(Quaternion(keyValuePairs[3]["x"], keyValuePairs[3]["w"], keyValuePairs[3]["z"], keyValuePairs[3]["y"]));
	TileType value = (TileType)keyValuePairs[1]["type"];
	if (level) level->mTileMap[key] = value;
	else room->mTileMap[key] = value;
}

void JsonParser::WriteRoomList(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	if (keyValuePairs.size() == 1) return;
	level->mRoomList[Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"])] = new Room((int)keyValuePairs[1]["type"],
		(int)keyValuePairs[1]["doorPositions"], (int)keyValuePairs[1]["primaryDoor"]);
}

void JsonParser::WriteGuardCount(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	level->mGuardCount = (int)keyValuePairs[0]["guardCount"];
}

void JsonParser::WriteCCTVCount(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	level->mCCTVCount = (int)keyValuePairs[0]["cctvCount"];
}

void JsonParser::WriteGuardPaths(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	if (keyValuePairs.size() == 1) return;
	level->mGuardPaths.push_back(std::vector<Vector3>());
	for (int i = 2; i < keyValuePairs.size(); i++) {
		level->mGuardPaths[level->mGuardPaths.size() - 1].push_back(Vector3(keyValuePairs[i]["x"], keyValuePairs[i]["y"], -keyValuePairs[i]["z"]));
	}
}

void JsonParser::WriteCCTVTransforms(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	if (keyValuePairs.size() == 1) return;
	Transform newTransform = Transform();
	newTransform.SetPosition(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]))
		.SetOrientation(Quaternion(keyValuePairs[3]["x"], keyValuePairs[3]["w"], keyValuePairs[3]["z"], keyValuePairs[3]["y"]));
	if (level) level->mCCTVTransforms.push_back(newTransform);
	else room->mCCTVTransforms.push_back(newTransform);
}

void JsonParser::WritePrisonPosition(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	level->mPrisonPosition = Vector3(keyValuePairs[1]["x"], keyValuePairs[1]["y"], -keyValuePairs[1]["z"]);
}

void JsonParser::WritePlayerStartTransforms(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	if (mPlayerCount < MAX_PLAYERS) {
		Matrix4 xRot = Matrix4::Rotation(keyValuePairs[3]["x"], Vector3(1, 0, 0));
		Matrix4 yRot = Matrix4::Rotation(keyValuePairs[3]["y"] - 180, Vector3(0, -1, 0));
		Matrix4 zRot = Matrix4::Rotation(-keyValuePairs[3]["z"], Vector3(0, 0, 1));
		Vector3 direction = yRot * xRot * zRot * Vector3(0, 0, 90);
		Transform newTransform = Transform();
		newTransform.SetPosition(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]))
			.SetOrientation(Quaternion::EulerAnglesToQuaternion(direction.x, direction.y, direction.z));
		level->mPlayerStartTransforms[mPlayerCount] = newTransform;
		mPlayerCount++;
	}
}

void JsonParser::WriteDirectionalLight(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	/*Light* newLight = (Light*)new DirectionLight(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]),
		Vector4(keyValuePairs[3]["x"], keyValuePairs[3]["y"], keyValuePairs[3]["z"], keyValuePairs[3]["w"]));
	level->mLights.push_back(newLight);*/
}

void JsonParser::WritePointlight(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	if (keyValuePairs.size() == 1) return;
	Light* newLight = (Light*)new PointLight(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]),
		Vector4(keyValuePairs[3]["x"], keyValuePairs[3]["y"], keyValuePairs[3]["z"], keyValuePairs[3]["w"]), keyValuePairs[1]["radius"]);
	if (level) level->mLights.push_back(newLight);
	else room->mLights.push_back(newLight);
}

void JsonParser::WriteSpotlight(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	if (keyValuePairs.size() == 1) return;
	Matrix4 xRot = Matrix4::Rotation(keyValuePairs[4]["x"], Vector3(1, 0, 0));
	Matrix4 yRot = Matrix4::Rotation(keyValuePairs[4]["y"] - 180, Vector3(0, -1, 0));
	Matrix4 zRot = Matrix4::Rotation(-keyValuePairs[4]["z"], Vector3(0, 0, 1));
	Vector3 direction = yRot * xRot * zRot * Vector3(0, 0, 90);
	Light* newLight = (Light*)new SpotLight(direction,
		Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]),
		Vector4(keyValuePairs[3]["x"], keyValuePairs[3]["y"], keyValuePairs[3]["z"], keyValuePairs[3]["w"]),
		keyValuePairs[1]["radius"], keyValuePairs[1]["angle"], 1.0f);
	if (level) level->mLights.push_back(newLight);
	else room->mLights.push_back(newLight);
}

void JsonParser::WriteItemPositions(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	Vector3 newPos = Vector3(keyValuePairs[1]["x"], keyValuePairs[1]["y"], -keyValuePairs[1]["z"]);
	if (level) level->mItemPositions.push_back(newPos);
	else room->mItemPositions.push_back(newPos);
}

void JsonParser::WriteVents(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	if (keyValuePairs.size() == 1) return;
	Vent* vent = new Vent();
	vent->GetTransform().SetPosition(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]))
		.SetOrientation(Quaternion::EulerAnglesToQuaternion(keyValuePairs[3]["x"], keyValuePairs[3]["y"], -keyValuePairs[3]["z"]));
	level->mVents.push_back(vent);
	level->mVentConnections.push_back(keyValuePairs[1]["connectedVentID"]);
}

void JsonParser::WriteHelipad(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	level->mHelipadPosition = Vector3(keyValuePairs[1]["x"], keyValuePairs[1]["y"], -keyValuePairs[1]["z"]);
}

void JsonParser::WriteDoors(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	if (keyValuePairs.size() == 1) return;
	InteractableDoor* door = new InteractableDoor();
	door->GetTransform().SetPosition(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]))
		.SetOrientation(Quaternion::EulerAnglesToQuaternion(keyValuePairs[3]["x"], keyValuePairs[3]["y"] - 180, -keyValuePairs[3]["z"]));
	if (level) level->mDoors.push_back(door);
	else room->mDoors.push_back(door);
}

void JsonParser::WritePrisonDoorPos(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	PrisonDoor* pDoor = new PrisonDoor();
	pDoor->GetTransform().SetPosition(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]))
		.SetOrientation(Quaternion::EulerAnglesToQuaternion(keyValuePairs[3]["x"], keyValuePairs[3]["y"] - 180, -keyValuePairs[3]["z"]));
	level->mPrisonDoor = pDoor;
}

void JsonParser::WriteDecorationTransforms(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room) {
	if (keyValuePairs.size() == 1) return;
	Transform value = Transform();
	value.SetPosition(Vector3(keyValuePairs[2]["x"], keyValuePairs[2]["y"], -keyValuePairs[2]["z"]))
		.SetOrientation(Quaternion(keyValuePairs[3]["x"], keyValuePairs[3]["w"], keyValuePairs[3]["z"], keyValuePairs[3]["y"]));
	DecorationType key = (DecorationType)keyValuePairs[1]["type"];
	if (level) {
		if (level->mDecorationMap.find(key) == level->mDecorationMap.end()) {
			level->mDecorationMap[key] = std::vector<Transform>();
		}
		level->mDecorationMap[key].push_back(value);
	}
	else {
		if (room->mDecorationMap.find(key) == room->mDecorationMap.end()) {
			room->mDecorationMap[key] = std::vector<Transform>();
		}
		room->mDecorationMap[key].push_back(value);
	}
}
