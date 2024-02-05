#pragma once
using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		enum RoomType {
			Medium,
			MAX
		};
		class Room {
		public:
			Room();
			~Room() {}
			RoomType GetType() const { return mType; }
			std::vector<Vector3> GetCCTVPositions() { return mCCTVPositions; }
			std::vector<Vector3> GetCCTVRotations() { return mCCTVRotations; }
		protected:
			RoomType mType;
			std::map<Vector3, GameObject*> mTileMap;
			//NavMesh
			//std::vector<Lights> mLights;
			std::vector<Vector3> mCCTVPositions;
			std::vector<Vector3> mCCTVRotations;
			std::vector<Vector3> mItemPositions;
		};
	}
}