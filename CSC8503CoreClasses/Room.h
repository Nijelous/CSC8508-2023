#pragma once
#include "LevelEnums.h"
using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class Light;

		class Room {
		public:
			Room() { mType = INVALID; }
			Room(int type);
			Room(std::string roomPath);
			~Room() {}
			RoomType GetType() const { return mType; }
			friend class JsonParser;
		protected:
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