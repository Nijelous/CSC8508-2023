#include "RenderObject.h"
#include "Mesh.h"

using namespace NCL::CSC8503;
using namespace NCL;

RenderObject::RenderObject(Transform* parentTransform, Mesh* mesh, Texture* albedo, Texture* normal, Shader* shader, float cullSphereRadius) {
	mTransform	= parentTransform;
	mMesh		= mesh;
	mAlbedoTex	= albedo;
	mNormalTex = normal;
	mShader	= shader;
	mColour	= Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	mCullSphereRadius = cullSphereRadius;
}

RenderObject::RenderObject(Transform* parentTransform, Mesh* mesh, Texture* albedo, Texture* normal, Shader* shader, Vector4 colour, float cullSphereRadius) {
	mTransform = parentTransform;
	mMesh = mesh;
	mAlbedoTex = albedo;
	mNormalTex = normal;
	mShader = shader;
	mColour = colour;
	mCullSphereRadius = cullSphereRadius;
}

RenderObject::~RenderObject() {

}