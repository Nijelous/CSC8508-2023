#pragma once
#include "../Recast/Include/Recast.h"
#include "../Detour/Include/DetourNavMesh.h"
#include "../Detour/Include/DetourNavMeshQuery.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		enum SamplePolyAreas
		{
			SAMPLE_POLYAREA_GROUND,
			SAMPLE_POLYAREA_WATER,
			SAMPLE_POLYAREA_ROAD,
			SAMPLE_POLYAREA_DOOR,
			SAMPLE_POLYAREA_GRASS,
			SAMPLE_POLYAREA_JUMP
		};
		enum SamplePolyFlags
		{
			SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
			SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
			SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
			SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
			SAMPLE_POLYFLAGS_DISABLED = 0x10,	// Disabled polygon
			SAMPLE_POLYFLAGS_ALL = 0xffff		// All abilities.
		};
		constexpr int x = 0;
		constexpr int y = 1;
		constexpr int z = 2;
		class RecastBuilder {
		public:
			RecastBuilder();
			~RecastBuilder();
			void BuildNavMesh(std::vector<GameObject*> objects);
			dtNavMeshQuery* GetNavMeshQuery() const { return mNavMeshQuery; }
			dtNavMesh* GetNavMesh() const { return mNavMesh; }
			bool HasSetSize() { return mSizeSet; }
		protected:
			bool InitialiseConfig(const float* bmin, const float* bmax);
			bool RasterizeInputPolygon(float* verts, const int vertCount, const unsigned int* tris, const int trisCount);
			bool FilterWalkableSurfaces();
			bool PartitionWalkableSurface();
			bool TraceContours();
			bool BuildPoly();
			bool BuildDetailPoly();
			bool CreateDetourData();

			unsigned char* mTriAreas;

			rcHeightfield* mSolid;
			rcCompactHeightfield* mCompHF;
			rcContourSet* mContSet;
			rcPolyMesh* mPolyMesh;
			rcConfig mConfig;
			rcPolyMeshDetail* mMeshDetail;
			dtNavMesh* mNavMesh;
			dtNavMeshQuery* mNavMeshQuery;

			float mCellSize = 0.3f;
			float mCellHeight = 0.2f;

			float mGuardMaxSlope = 10.0f;
			float mGuardRadius = 3.0f;
			float mGuardHeight = 3.0f;
			float mGuardMaxClimb = 0.1f;

			float mMaxEdgeLength = 12.0f;
			float mMaxEdgeError = 1.3f;

			int mVertsPerPoly = 6;
			int mMinRegionSize = 8;
			int mMergedRegionSize = 20;

			int mSampleDistance = 6;
			int mMaxSampleError = 1;

			bool mSizeSet = false;

			void cleanup();
		};
	}
}
