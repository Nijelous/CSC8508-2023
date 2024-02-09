#include "RecastBuilder.h"
#include "RenderObject.h"
#include "GameObject.h"
#include "../OpenGLRendering/OGLRenderer.h"

using namespace NCL::CSC8503;

RecastBuilder::RecastBuilder() {
	mTriAreas = nullptr;
	mSolid = nullptr;
	mCompHF = nullptr;
	mContSet = nullptr;
	mPolyMesh = nullptr;
	mMeshDetail = nullptr;
	mNavMesh = nullptr;
}

RecastBuilder::~RecastBuilder() {
	cleanup();
}

void RecastBuilder::BuildNavMesh(GameObject* mesh) {
	if (!mesh) return;

	cleanup();

	const float* bmin;
	const float* bmax;
	const std::vector<Vector3> verts = mesh->GetRenderObject()->GetMesh()->GetPositionData();
	const int vertCount = mesh->GetRenderObject()->GetMesh()->GetVertexCount();
	const int* tris;
	const int trisCount = mesh->GetRenderObject()->GetMesh()->GetPrimitiveCount();
	for (int i = 0; i < vertCount; i++) {
		std::cout << verts[i] << "\n";
	}
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
}
