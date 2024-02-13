#pragma once
#include "Room.h"
using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		constexpr int MAX_PLAYERS = 4;

		class Vent;

		class Level {
		public:
			Level(std::string levelPath);
			~Level();
			std::map<Vector3, TileType> GetTileMap() const { return mTileMap; }
			std::map<Vector3, Room*> GetRooms() const { return mRoomList; }
			std::vector<std::vector<Vector3>> GetGuardPaths() const { return mGuardPaths; }
			int GetGuardCount() const { return mGuardCount; }
			std::vector<Transform> GetCCTVTransforms() const { return mCCTVTransforms; }
			int GetCCTVCount() const { return mCCTVCount; }
			Vector3 GetPrisonPosition() const { return mPrisonPosition; }
			Transform GetPlayerStartTransform(int player) const { if (player < 0 || player >= 4) return Transform(); return mPlayerStartTransforms[player]; }
			std::vector<Light*> GetLights() const { return mLights; }
			std::vector<Vector3> GetItemPositions() const { return mItemPositions; }
			std::vector<Vent*> GetVents() const { return mVents; }
			Vector3 GetHelipadPosition() const { return mHelipadPosition; }

			friend class JsonParser;
		protected:
			std::string mLevelName;
			std::map<Vector3, TileType> mTileMap;
			std::map<Vector3, Room*> mRoomList;
			std::vector<std::vector<Vector3>> mGuardPaths;
			int mGuardCount;
			std::vector<Transform> mCCTVTransforms;
			int mCCTVCount;
			Vector3 mPrisonPosition;
			Transform* mPlayerStartTransforms;
			std::vector<Light*> mLights;
			//NavMesh
			std::vector<Vector3> mItemPositions;
			std::vector<Vent*> mVents;
			std::vector<int> mVentConnections;
			Vector3 mHelipadPosition;
		};
	}
}

