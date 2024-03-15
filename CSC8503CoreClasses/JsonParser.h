#pragma once
#include "Level.h"

namespace NCL {
	namespace CSC8503 {
		enum ParserVariables {
			SetRoomType,
			RoomDoorPos,
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
			Doors,
			PrisonDoorPos,
			DecorationTransforms
		};
		class JsonParser {
		public:
			JsonParser();
			void ParseJson(std::string JSON, Level* level, Room* room);
		protected:
			void WriteValue(bool writingValue, std::vector<std::unordered_map<std::string, float>>* keyValuePairs,
				std::string key, std::string* value, int indents, int maxIndents);
			int mPlayerCount = 0;
			std::unordered_map<int, std::function<void(std::vector<std::unordered_map<std::string, float>>&, Level*, Room*)>> mVariableWriteMap;
			void WriteRoomType(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WriteRoomDoorPos(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WriteTileMap(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WriteRoomList(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WriteGuardCount(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WriteCCTVCount(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WriteGuardPaths(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WriteCCTVTransforms(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WritePrisonPosition(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WritePlayerStartTransforms(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WriteDirectionalLight(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WritePointlight(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WriteSpotlight(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WriteItemPositions(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WriteVents(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WriteHelipad(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WriteDoors(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WritePrisonDoorPos(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WriteDecorationTransforms(std::vector<std::unordered_map<std::string, float>>& keyValuePairs, Level* level, Room* room);
		};
	}
}
