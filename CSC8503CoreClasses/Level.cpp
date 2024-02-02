#include "Level.h"
#include "Room.h"
#include "Vent.h"

using namespace NCL::CSC8503;

Level::Level(int levelID) {
	mLevelID = levelID;
	//std::map<Vector3, Tile> mTileMap;
	mRoomList = std::map<Vector3, Room>();
	mGuardPaths = std::vector<std::vector<Vector3>>();
	mGuardCount = 0;
	mCCTVPositions = std::vector<Vector3>();
	mCCTVCount = 0;
	mPrisonPosition = Vector3(0, 0, 0);
	mPlayerStartPositions = std::make_unique_for_overwrite<Vector3[]>(4);
	//mLights = std::vector<Vector3>();
	//NavMesh
	//mItemPositions = std::vector<Vector3>();
	mVents = std::vector<Vent*>();
}
