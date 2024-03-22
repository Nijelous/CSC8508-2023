#include "RenderObject.h"
#include "Mesh.h"

using namespace NCL::CSC8503;
using namespace NCL;

RenderObject::RenderObject(Transform* parentTransform, Mesh* mesh, Texture* albedoTex, Texture* normalTex, Shader* shader, float cullSphereRadius) {
	mBuffer = nullptr;

	mTransform = parentTransform;
	mMesh = mesh;
	mAlbedoTex = albedoTex;
	mNormalTex = normalTex;
	mShader = shader;
	mColour = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	mCullSphereRadius = cullSphereRadius;
	mSqDistToCam = FLT_MAX;
	mAnimationObject = nullptr;

	mCurrentFrame = 0;

	vector<uint64_t>  mMatTextures = {};
	vector<Matrix4> mFrameMatrices = {};

}

RenderObject::RenderObject(Transform* parentTransform, Mesh* mesh, Texture* albedoTex, Texture* normalTex, Shader* shader, Vector4 colour, float cullSphereRadius) {

	mTransform = parentTransform;
	mMesh = mesh;
	mAlbedoTex = albedoTex;
	mNormalTex = normalTex;
	mShader = shader;
	mColour = colour;
	mCullSphereRadius = cullSphereRadius;
	mSqDistToCam = FLT_MAX;
	mAnimationObject = nullptr;

	mCurrentFrame = 0;
	vector<uint64_t>  mMatTextures = {};
	vector<Matrix4> mFrameMatrices = {};
}

