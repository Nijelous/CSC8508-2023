#pragma once
#include "../Recast/Include/Recast.h"
#include "../Detour/Include/DetourNavMesh.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class RecastBuilder {
		public:
			RecastBuilder();
			~RecastBuilder();
			void BuildNavMesh(GameObject* mesh);
		protected:
			unsigned char* mTriAreas;
			rcHeightfield* mSolid;
			rcCompactHeightfield* mCompHF;
			rcContourSet* mContSet;
			rcPolyMesh* mPolyMesh;
			rcConfig mConfig;
			rcPolyMeshDetail* mMeshDetail;
			dtNavMesh* mNavMesh;

			void cleanup();
		};
	}
}
