#pragma once
#include "Room.h"
using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class Vent;
		class PrisonDoor;
		class Level {
		public:
			Level(std::string levelPath);
			~Level();
			const std::unordered_map<Transform, TileType>& GetTileMap() { return mTileMap; }
			const std::unordered_map<Vector3, Room*>& GetRooms() { return mRoomList; }
			std::vector<std::vector<Vector3>> GetGuardPaths() const { return mGuardPaths; }
			int GetGuardCount() const { return mGuardCount; }
			std::vector<Transform> GetCCTVTransforms() const { return mCCTVTransforms; }
			int GetCCTVCount() const { return mCCTVCount; }
			Vector3 GetPrisonPosition() const { return mPrisonPosition; }
			Transform GetPlayerStartTransform(int player) const { if (player < 0 || player >= 4) return Transform(); return mPlayerStartTransforms[player]; }
			std::vector<Light*> GetLights() const { return mLights; }
			std::vector<Vector3> GetItemPositions() const { return mItemPositions; }
			std::vector<Vent*> GetVents() const { return mVents; }
			std::vector<int> GetVentConnections() const { return mVentConnections; }
			Vector3 GetHelipadPosition() const { return mHelipadPosition; }
			std::vector<Door*> GetDoors() const { return mDoors; }
			PrisonDoor* GetPrisonDoor() const { return mPrisonDoor; }
			const std::unordered_map<DecorationType, std::vector<Transform>>& GetDecorationMap() { return mDecorationMap; }

			friend class JsonParser;
		protected:
			std::string mLevelName;
			std::unordered_map<Transform, TileType> mTileMap;
			std::unordered_map<Vector3, Room*> mRoomList;
			std::vector<std::vector<Vector3>> mGuardPaths;
			int mGuardCount;
			std::vector<Transform> mCCTVTransforms;
			int mCCTVCount;
			Vector3 mPrisonPosition;
			Transform* mPlayerStartTransforms;
			std::vector<Light*> mLights;
			std::vector<Vector3> mItemPositions;
			std::vector<Vent*> mVents;
			std::vector<int> mVentConnections;
			Vector3 mHelipadPosition;
			std::vector<Door*> mDoors;
			PrisonDoor* mPrisonDoor;
			std::unordered_map<DecorationType, std::vector<Transform>> mDecorationMap;
		};
	}
}

