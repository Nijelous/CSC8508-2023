#pragma once
#include "Room.h"
using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class Vent;
		class Light;
		class Room;

		class Level {
		public:
			Level(std::string levelPath);
			~Level();
			int GetGuardCount() const { return mGuardCount; }
			int GetCCTVCount() const { return mCCTVCount; }
			Vector3 GetPrisonPosition() const { return mPrisonPosition; }
			Vector3 GetPlayerStartPosition(int player) { return mPlayerStartPositions[player]; }
			std::map<Vector3, TileType> GetTileMap() const { return mTileMap; }
			friend class JsonParser;
		protected:
			std::string mLevelName;
			std::map<Vector3, TileType> mTileMap;
			std::map<Vector3, Room> mRoomList;
			std::vector<std::vector<Vector3>> mGuardPaths;
			int mGuardCount;
			std::vector<Matrix4> mCCTVTransforms;
			int mCCTVCount;
			Vector3 mPrisonPosition;
			Vector3* mPlayerStartPositions;
			std::vector<Light*> mLights;
			//NavMesh
			std::vector<Vector3> mItemPositions;
			std::vector<Vent*> mVents;
			std::vector<int> mVentConnections;
		};
	}
}

