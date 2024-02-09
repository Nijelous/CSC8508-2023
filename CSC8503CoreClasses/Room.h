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
			~Room();
			RoomType GetType() const { return mType; }
			std::map<Vector3, TileType> GetTileMap() const { return mTileMap; }
			std::vector<Light*> GetLights() const { return mLights; }
			std::vector<Matrix4> GetCCTVTransforms() const { return mCCTVTransforms; }
			std::vector<Vector3> GetItemPositions() const { return mItemPositions; }
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