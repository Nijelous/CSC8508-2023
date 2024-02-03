#pragma once
using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class Vent;
		class Room;
		class GameObject;
		class Level {
		public:
			Level(int levelID);
			~Level() {}
			int GetGuardCount() const { return mGuardCount; }
			int GetCCTVCount() const { return mCCTVCount; }
			Vector3 GetPrisonPosition() const { return mPrisonPosition; }
			Vector3 GetPlayerStartPosition(int player) { return mPlayerStartPositions[player]; }
		protected:
			int mLevelID;
			std::map<Vector3, GameObject*> mTileMap;
			std::map<Vector3, Room> mRoomList;
			std::vector<std::vector<Vector3>> mGuardPaths;
			int mGuardCount;
			std::vector<Vector3> mCCTVPositions;
			std::vector<Vector3> mCCTVRotations;
			int mCCTVCount;
			Vector3 mPrisonPosition;
			std::unique_ptr<Vector3[]> mPlayerStartPositions;
			//std::vector<Lights> mLights;
			//NavMesh
			std::vector<Vector3> mItemPositions;
			std::vector<Vent*> mVents;
		};
	}
}

