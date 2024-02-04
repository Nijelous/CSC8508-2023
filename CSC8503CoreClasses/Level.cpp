#include "Level.h"
#include "Vent.h"
#include "GameObject.h"

using namespace NCL::CSC8503;

Level::Level(int levelID) {
	mLevelID = levelID;
	std::map<Vector3, GameObject*> mTileMap;
	mRoomList = std::map<Vector3, Room>();
	mGuardPaths = std::vector<std::vector<Vector3>>();
	mGuardCount = 0;
	mCCTVPositions = std::vector<Vector3>();
	mCCTVCount = 0;
	mPrisonPosition = Vector3(0, 0, 0);
	mPlayerStartPositions = new Vector3[4];
	//mLights = std::vector<Light*>();
	//NavMesh
	mItemPositions = std::vector<Vector3>();
	mVents = std::vector<Vent*>();
}
