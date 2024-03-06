#pragma once
#include "LevelEnums.h"
#include "BaseLight.h"
#include "Transform.h"
#include "Door.h"
#include <unordered_map>

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class GameObject;

		class Room {
		public:
			Room() { mType = INVALID; }
			Room(int type);
			Room(std::string roomPath);
			~Room();
			RoomType GetType() const { return mType; }
			std::unordered_map<Transform, TileType> GetTileMap() const { return mTileMap; }
			std::vector<Light*> GetLights() const { return mLights; }
			std::vector<Transform> GetCCTVTransforms() const { return mCCTVTransforms; }
			std::vector<Vector3> GetItemPositions() const { return mItemPositions; }
			std::vector<Door*> GetDoors() const { return mDoors; }
			friend class JsonParser;
		protected:
			std::string mRoomName;
			RoomType mType;
			std::unordered_map<Transform, TileType> mTileMap;
			std::vector<Light*> mLights;
			std::vector<Transform> mCCTVTransforms;
			std::vector<Vector3> mItemPositions;
			std::vector<Door*> mDoors;
		};
	}
}