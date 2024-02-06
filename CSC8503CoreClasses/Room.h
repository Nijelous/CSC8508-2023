#pragma once
#include "LevelEnums.h"
using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class Light;

		class Room {
		public:
			Room(){}
			Room(int type);
			Room(std::string roomPath);
			~Room() {}
			RoomType GetType() const { return mType; }
		protected:
			void WriteVariable(std::vector<std::map<std::string, float>>& keyValuePairs);
			void WriteValue(bool writingValue, std::vector<std::map<std::string, float>>* keyValuePairs, std::string key, std::string* value, int indents, int maxIndents);

			std::string mRoomName;
			RoomType mType;
			std::map<Vector3, TileType> mTileMap;
			//NavMesh
			std::vector<Light*> mLights;
			std::vector<Matrix4> mCCTVTransforms;
			std::vector<Vector3> mItemPositions;
		};
	}
}