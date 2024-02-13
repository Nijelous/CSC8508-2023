#pragma once
#include "Level.h"

namespace NCL {
	namespace CSC8503 {
		enum ParserVariables {
			SetRoomType,
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
			Helipad
		};
		class JsonParser {
		public:
			JsonParser(){}
			void ParseJson(std::string JSON, Level* level, Room* room);
		protected:
			void WriteVariable(std::vector<std::map<std::string, float>>& keyValuePairs, Level* level, Room* room);
			void WriteValue(bool writingValue, std::vector<std::map<std::string, float>>* keyValuePairs, std::string key, std::string* value, int indents, int maxIndents);
			int mPlayerCount = 0;
		};
	}
}
