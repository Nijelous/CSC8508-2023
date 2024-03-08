#include "RecastBuilder.h"
#include "RenderObject.h"
#include "GameObject.h"
#include "../OpenGLRendering/OGLRenderer.h"
#include "../Detour/Include/DetourNavMeshBuilder.h"
#include "../CSC8503/LevelManager.h"
#include <thread>

using namespace NCL::CSC8503;

RecastBuilder::RecastBuilder() {
	mTriAreas = nullptr;
	mSolid = nullptr;
	mCompHF = nullptr;
	mContSet = nullptr;
	mPolyMesh = nullptr;
	mMeshDetail = nullptr;
	mNavMesh = nullptr;
	mNavMeshQuery = nullptr;
}

RecastBuilder::~RecastBuilder() {
	cleanup();
}

void RecastBuilder::BuildNavMesh(std::vector<GameObject*> objects) {
	mSizeSet = false;
	if (objects.empty()) return;

	cleanup();

	// Translating the meshes into useable values for Recast
	float* bmin = new float[3] {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	float* bmax = new float[3] {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
	int currentTriCount = 0;
	int vertCount = 0;
	int trisCount = 0;
	for (int i = 0; i < objects.size(); i++) {
		vertCount += objects[i]->GetRenderObject()->GetMesh()->GetVertexCount();
		trisCount += objects[i]->GetRenderObject()->GetMesh()->GetPrimitiveCount();
	}
	float* verts = new float[vertCount * 3];
	unsigned int* tris = new unsigned int[trisCount * 3];
	int vCount = 0;
	int tCount = 0;
	vertCount = 0;
	for (int i = 0; i < objects.size(); i++) {
		int lastVertCount = vertCount;
		std::vector<Vector3> vertsVector = objects[i]->GetRenderObject()->GetMesh()->GetPositionData();
		for (int j = 0; j < vertsVector.size(); j++) {
			vertsVector[j] *= objects[i]->GetTransform().GetScale();
			vertsVector[j] = (objects[i]->GetTransform().GetOrientation() * Quaternion(vertsVector[j], 0) * objects[i]->GetTransform().GetOrientation().Conjugate()).ToVector3();
			vertsVector[j] += objects[i]->GetTransform().GetPosition();
			verts[vCount++] = vertsVector[j].x;
			verts[vCount++] = vertsVector[j].y;
			verts[vCount++] = vertsVector[j].z;
			bmin[x] = std::min(bmin[x], verts[vCount + (x-3)]);
			bmin[y] = std::min(bmin[y], verts[vCount + (y-3)]);
			bmin[z] = std::min(bmin[z], verts[vCount + (z-3)]);
			bmax[x] = std::max(bmax[x], verts[vCount + (x-3)]);
			bmax[y] = std::max(bmax[y], verts[vCount + (y-3)]);
			bmax[z] = std::max(bmax[z], verts[vCount + (z-3)]);
		}

		std::vector<unsigned int> trisVector = objects[i]->GetRenderObject()->GetMesh()->GetIndexData();
		for (int j = 0; j < trisVector.size(); j++) {
			tris[tCount] = trisVector[j] + lastVertCount;
			tCount++;
		}
		vertCount += objects[i]->GetRenderObject()->GetMesh()->GetVertexCount();
	}
	LevelManager::GetLevelManager()->GetPhysics()->SetNewBroadphaseSize(Vector3(bmax[x] - bmin[x], bmax[y] - bmin[y], bmax[z] - bmin[z]));
	mSizeSet = true;
	if (!InitialiseConfig(bmin, bmax)) return;
	if (!RasterizeInputPolygon(verts, vertCount, tris, trisCount)) return;
	if (!FilterWalkableSurfaces()) return;
	if (!PartitionWalkableSurface()) return;
	if (!TraceContours()) return;
	if (!BuildPoly()) return;
	if (!BuildDetailPoly()) return;
	if (!CreateDetourData()) return;
	delete[] bmin;
	delete[] bmax;
	delete[] verts;
	delete[] tris;
	return;
}

bool RecastBuilder::InitialiseConfig(const float* bmin, const float* bmax) {
	memset(&mConfig, 0, sizeof(mConfig));

	mConfig.cs = mCellSize;
	mConfig.ch = mCellHeight;
	mConfig.walkableSlopeAngle = mGuardMaxSlope;
	mConfig.walkableHeight = mConfig.ch != 0 ? (int)ceilf(mGuardHeight / mConfig.ch) : 0;
	mConfig.walkableClimb = mConfig.ch != 0 ? (int)floorf(mGuardMaxClimb / mConfig.ch) : 0;
	mConfig.walkableRadius = mConfig.cs != 0 ? (int)ceilf(mGuardRadius / mConfig.cs) : 0;
	mConfig.maxEdgeLen = mConfig.cs != 0 ? (int)(mMaxEdgeLength / mConfig.cs) : 0;
	mConfig.maxSimplificationError = mMaxEdgeError;
	mConfig.minRegionArea = (int)rcSqr(mMinRegionSize);		// Note: area = size*size
	mConfig.mergeRegionArea = (int)rcSqr(mMergedRegionSize);	// Note: area = size*size
	mConfig.maxVertsPerPoly = mVertsPerPoly;
	mConfig.detailSampleDist = mSampleDistance < 0.9f ? 0 : mConfig.cs * mSampleDistance;
	mConfig.detailSampleMaxError = mConfig.ch * mMaxSampleError;

	rcVcopy(mConfig.bmin, bmin);
	rcVcopy(mConfig.bmax, bmax);
	rcCalcGridSize(mConfig.bmin, mConfig.bmax, mConfig.cs, &mConfig.width, &mConfig.height);
	return true;
}

bool RecastBuilder::RasterizeInputPolygon(float* verts, const int vertCount, const unsigned int* tris, const int trisCount) {
	mSolid = rcAllocHeightfield();
	if (!mSolid) {
		std::cout << "Recast Error: Out of memory 'mSolid'\n";
		return false;
	}
	if (!rcCreateHeightfield(nullptr, *mSolid, mConfig.width, mConfig.height, mConfig.bmin, mConfig.bmax, mConfig.cs, mConfig.ch)) {
		std::cout << "Recast Error: Could not create solid heightfield\n";
		return false;
	}

	mTriAreas = new unsigned char[trisCount];
	if (!mTriAreas) {
		std::cout << "Recast Error: Out of memory 'mTriAreas'\n";
		return false;
	}

	memset(mTriAreas, 0, trisCount * sizeof(unsigned char));
	rcMarkWalkableTriangles(nullptr, mConfig.walkableSlopeAngle, verts, vertCount, tris, trisCount, mTriAreas);
	if (!rcRasterizeTriangles(nullptr, verts, vertCount, tris, mTriAreas, trisCount, *mSolid, mConfig.walkableClimb)) {
		std::cout << "Recast Error: Could not rasterize the triangles\n";
		return false;
	}
	return true;
}

bool RecastBuilder::FilterWalkableSurfaces() {
	std::thread lowHangingWalkable([this] {rcFilterLowHangingWalkableObstacles(nullptr, mConfig.walkableClimb, *mSolid); });
	std::thread ledgeSpans([this] {rcFilterLedgeSpans(nullptr, mConfig.walkableHeight, mConfig.walkableClimb, *mSolid); });
	std::thread walkableLowHeight([this] {rcFilterWalkableLowHeightSpans(nullptr, mConfig.walkableHeight, *mSolid); });
	lowHangingWalkable.join();
	ledgeSpans.join();
	walkableLowHeight.join();
	return true;
}

bool RecastBuilder::PartitionWalkableSurface() {
	mCompHF = rcAllocCompactHeightfield();
	if (!mCompHF) {
		std::cout << "Recast Error: Out of memory 'mCompHF'\n";
		return false;
	}
	if (!rcBuildCompactHeightfield(nullptr, mConfig.walkableHeight, mConfig.walkableClimb, *mSolid, *mCompHF)) {
		std::cout << "Recast Error: Could not build compact data\n";
		return false;
	}

	if (!rcErodeWalkableArea(nullptr, mConfig.walkableRadius, *mCompHF)) {
		std::cout << "Recast Error: Could not erode\n";
		return false;
	}

	if (!rcBuildRegionsMonotone(nullptr, *mCompHF, 0, mConfig.minRegionArea, mConfig.mergeRegionArea)) {
		std::cout << "Recast Error: Could not build NavMesh regions\n";
		return false;
	}
	return true;
}

bool RecastBuilder::TraceContours() {
	mContSet = rcAllocContourSet();
	if (!mContSet) {
		std::cout << "Recast Error: Out of memory 'mContSet'\n";
		return false;
	}

	if (!rcBuildContours(nullptr, *mCompHF, mConfig.maxSimplificationError, mConfig.maxEdgeLen, *mContSet)) {
		std::cout << "Recast Error: Could not create contours\n";
		return false;
	}
	return true;
}

bool RecastBuilder::BuildPoly() {
	mPolyMesh = rcAllocPolyMesh();
	if (!mPolyMesh) {
		std::cout << "Recast Error: Out of memory 'mPolyMesh'\n";
		return false;
	}
	if (!rcBuildPolyMesh(nullptr, *mContSet, mConfig.maxVertsPerPoly, *mPolyMesh)) {
		std::cout << "Recast Error: Could not triangulate contours\n";
		return false;
	}
	return true;
}

bool RecastBuilder::BuildDetailPoly() {
	mMeshDetail = rcAllocPolyMeshDetail();
	if (!mMeshDetail) {
		std::cout << "Recast Error: Out of memory 'mMeshDetail'\n";
		return false;
	}

	if (!rcBuildPolyMeshDetail(nullptr, *mPolyMesh, *mCompHF, mConfig.detailSampleDist, mConfig.detailSampleMaxError, *mMeshDetail)) {
		std::cout << "Recast Error: Could not build detail mesh\n";
		return false;
	}
	return true;
}

bool RecastBuilder::CreateDetourData() {
	if (mConfig.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		unsigned char* navData = 0;
		int navDataSize = 0;

		// Update poly flags from areas.
		for (int i = 0; i < mPolyMesh->npolys; ++i) {
			if (mPolyMesh->areas[i] == RC_WALKABLE_AREA)
				mPolyMesh->areas[i] = SAMPLE_POLYAREA_GROUND;

			if (mPolyMesh->areas[i] == SAMPLE_POLYAREA_GROUND ||
				mPolyMesh->areas[i] == SAMPLE_POLYAREA_GRASS ||
				mPolyMesh->areas[i] == SAMPLE_POLYAREA_ROAD)
			{
				mPolyMesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
			}
			else if (mPolyMesh->areas[i] == SAMPLE_POLYAREA_WATER)
			{
				mPolyMesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
			}
			else if (mPolyMesh->areas[i] == SAMPLE_POLYAREA_DOOR)
			{
				mPolyMesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
			}
		}

		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = mPolyMesh->verts;
		params.vertCount = mPolyMesh->nverts;
		params.polys = mPolyMesh->polys;
		params.polyAreas = mPolyMesh->areas;
		params.polyFlags = mPolyMesh->flags;
		params.polyCount = mPolyMesh->npolys;
		params.nvp = mPolyMesh->nvp;
		params.detailMeshes = mMeshDetail->meshes;
		params.detailVerts = mMeshDetail->verts;
		params.detailVertsCount = mMeshDetail->nverts;
		params.detailTris = mMeshDetail->tris;
		params.detailTriCount = mMeshDetail->ntris;
		params.walkableHeight = mGuardHeight;
		params.walkableRadius = mGuardRadius;
		params.walkableClimb = mGuardMaxClimb;
		rcVcopy(params.bmin, mPolyMesh->bmin);
		rcVcopy(params.bmax, mPolyMesh->bmax);
		params.cs = mConfig.cs;
		params.ch = mConfig.ch;
		params.buildBvTree = true;

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
			std::cout << "Detour Error: Could not build Detour navmesh\n";
			return false;
		}

		mNavMesh = dtAllocNavMesh();
		if (!mNavMesh)
		{
			dtFree(navData);
			std::cout << "Detour Error: Could not create Detour navmesh\n";
			return false;
		}

		dtStatus status;

		status = mNavMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		if (dtStatusFailed(status))
		{
			dtFree(navData);
			std::cout << "Detour Error: Could not init Detour navmesh\n";
			return false;
		}
		mNavMeshQuery = dtAllocNavMeshQuery();
		if (!mNavMeshQuery) {
			std::cout << "Detour Error: Could not create Detour navmesh query\n";
			return false;
		}
		status = mNavMeshQuery->init(mNavMesh, 2048);
		if (dtStatusFailed(status))
		{
			std::cout << "Detour Error: Could not init Detour navmesh query\n";
			return false;
		}
	}
	return true;
}

void RecastBuilder::cleanup() {
	delete[] mTriAreas;
	mTriAreas = NULL;
	rcFreeHeightField(mSolid);
	mSolid = NULL;
	rcFreeCompactHeightfield(mCompHF);
	mCompHF = NULL;
	rcFreeContourSet(mContSet);
	mContSet = NULL;
	rcFreePolyMesh(mPolyMesh);
	mPolyMesh = NULL;
	rcFreePolyMeshDetail(mMeshDetail);
	mMeshDetail = NULL;
	dtFreeNavMesh(mNavMesh);
	mNavMesh = NULL;
	dtFreeNavMeshQuery(mNavMeshQuery);
	mNavMeshQuery = NULL;
}
