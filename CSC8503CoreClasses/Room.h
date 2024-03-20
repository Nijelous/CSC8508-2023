#pragma once
#include "LevelEnums.h"
#include "Transform.h"
#include "BaseLight.h"
#include <unordered_map>

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class Door;
		class Room {
		public:
			Room() { mType = INVALID; }
			Room(int type, int doorPos, int primaryDoor);
			Room(std::string roomPath);
			~Room();
			std::string GetName() const { return mRoomName; }
			RoomType GetType() const { return mType; }
			std::unordered_map<Transform, TileType> GetTileMap() const { return mTileMap; }
			std::vector<Light*> GetLights() const { return mLights; }
			std::vector<Transform> GetCCTVTransforms() const { return mCCTVTransforms; }
			std::vector<Vector3> GetItemPositions() const { return mItemPositions; }
			std::vector<Door*> GetDoors() const { return mDoors; }
			int GetDoorConfig() const { return mDoorConfig; }
			int GetPrimaryDoor() const { return mPrimaryDoor; }
			const std::unordered_map<DecorationType, std::vector<Transform>>& GetDecorationMap() { return mDecorationMap; }

			friend class JsonParser;
		protected:
			std::string mRoomName;
			RoomType mType;
			int mDoorConfig;
			int mPrimaryDoor;
			std::unordered_map<Transform, TileType> mTileMap;
			std::vector<Light*> mLights;
			std::vector<Transform> mCCTVTransforms;
			std::vector<Vector3> mItemPositions;
			std::vector<Door*> mDoors;
			std::unordered_map<DecorationType, std::vector<Transform>> mDecorationMap;
		};
	}
}