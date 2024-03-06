#include "RenderObject.h"
#include "Mesh.h"

using namespace NCL::CSC8503;
using namespace NCL;

#ifdef USEGL

RenderObject::RenderObject(Transform* parentTransform, Mesh* mesh, Texture* albedoTex, Texture* normalTex, Shader* shader, float cullSphereRadius) {

	mTransform = parentTransform;
	mMesh = mesh;
	mAlbedoTex = albedoTex;
	mNormalTex = normalTex;
	mShader = shader;
	mColour = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	mCullSphereRadius = cullSphereRadius;
	mSqDistToCam = FLT_MAX;

	mCurrentFrame = 0;

	vector<GLuint>  mMatTextures = {};
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

	mCurrentFrame = 0;
	vector<GLuint>  mMatTextures = {};
	vector<Matrix4> mFrameMatrices = {};
}

RenderObject::~RenderObject() {

}

#endif

#ifdef USEPROSPERO
void RenderObject::SetAnimation(MeshAnimation& inAnim) {
	anim = &inAnim;

	skeleton.resize(anim->GetJointCount());
}

void RenderObject::UpdateAnimation(float dt) {
	if (!mesh || !anim) {
		return;
	}
	animTime -= dt;

	if (animTime <= 0) {
		currentAnimFrame++;
		animTime += anim->GetFrameTime();
		currentAnimFrame = (currentAnimFrame++) % anim->GetFrameCount();

		std::vector<Matrix4>const& inverseBindPose = mesh->GetInverseBindPose();

		if (inverseBindPose.size() != anim->GetJointCount()) {
			//oh no
			return;
		}

		const Matrix4* joints = anim->GetJointData(currentAnimFrame);

		for (int i = 0; i < skeleton.size(); ++i) {
			skeleton[i] = joints[i] * inverseBindPose[i];
		}
	}
}
#endif