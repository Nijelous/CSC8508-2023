#pragma once

#ifdef USEPROSPERO
#include <agc.h>
#include "AGCTexture.h"
#endif

#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"

#include "Transform.h"
#include "AnimationObject.h"
#include "Buffer.h"

#ifdef USEGL
#include <glad/gl.h>
#endif

typedef unsigned int GLuint;

namespace NCL {
	using namespace NCL::Rendering;

	namespace CSC8503 {
		class Transform;
		using namespace Maths;

		class RenderObject
		{
		public:
			RenderObject(Transform* inTransform, Mesh* inMesh, Texture* albedoTex, Texture* normalTex, Shader* inShader, float inCullSphereRadius);
			RenderObject(Transform* inTransform, Mesh* inMesh, Texture* albedoTex, Texture* normalTex, Shader* inShader, Vector4 inColour, float inCullSphereRadius);
			~RenderObject() {}

			void SetAlbedoTexture(Texture* t) {
				mAlbedoTex = t;
			}

			void SetNormalTexture(Texture* t) {
				mNormalTex = t;
			}

			Buffer* GetGPUBuffer() const {
				return mBuffer;
			}

			void SetGPUBuffer(Buffer* b) {
				mBuffer = b;
			}

			Texture* GetAlbedoTexture() const
			{
				return mAlbedoTex;
			}

			Texture* GetNormalTexture() const {
				return mNormalTex;
			}

			Mesh* GetMesh() const {
				return mMesh;
			}

			Transform* GetTransform() const {
				return mTransform;
			}

			Shader* GetShader() const {
				return mShader;
			}

			void SetColour(const Vector4& c) {
				mColour = c;
			}

			Vector4 GetColour() const {
				return mColour;
			}

			void SetSqDistToCam(const Vector3& camPos) {
				mSqDistToCam = (camPos - mTransform->GetPosition()).LengthSquared();
			}

			void SetSqDistToCam(float sqDist) {
				mSqDistToCam = sqDist;
			}

			static bool CompareBySqCamDist(const RenderObject* a, const RenderObject* b) {
				return(a->mSqDistToCam < b->mSqDistToCam) ? true : false;
			}

			static bool CompareByMesh(const RenderObject* a, const RenderObject* b)	{
				return(a->GetMesh() < b->GetMesh()) ? true : false;
			}

			void SetOutlined(bool outlined) {
				mOutlined = outlined;
			}

			bool GetOutlined() const {
				return mOutlined;
			}

			void SetIsInstanced(bool isInstanced) {
				mIsInstanced = isInstanced;
			}

			bool IsInstanced() const {
				return mIsInstanced;
			}


			void SetCurrentFrame(int currentFrame) {
				mCurrentFrame = currentFrame;
			}

			int GetCurrentFrame() const {
				return mCurrentFrame;
			}
#ifdef USEGL

			void SetMatTextures(vector<int> matTextures) {
				mMatTextures = matTextures;
			}

			vector<int>  GetMatTextures() const {
				return mMatTextures;
			}
#endif

#ifdef USEPROSPERO

			void SetMatTextures(vector<int> matTextures) {
				mMatTextures = matTextures;
			}

			vector<int>  GetMatTextures() const {
				return mMatTextures;
			}

			int GetIgnoredSubmeshID() const {
				return ignoredSubMeshID;
			}

			void SetIgnoredSubmeshID(int x) {
				ignoredSubMeshID = x;
			}

#endif

			void SetFrameMatricesVec(std::vector<std::vector<Matrix4>> frameMatrices) {
				mFrameMatricesVec = frameMatrices;
			}

			std::vector<std::vector<Matrix4>> const&  GetFrameMatricesVec() const {
				return mFrameMatricesVec;
			}

			AnimationObject* GetAnimationObject() const {
				return mAnimationObject;
			}

			void SetAnimationObject(AnimationObject* newObject) {
				mAnimationObject = newObject;
			}

			float GetCullSphereRadius() {
				return mCullSphereRadius;
			}

		protected:
			Buffer* mBuffer;
			Mesh* mMesh;
			Texture* mAlbedoTex;
			Texture* mNormalTex;
			Shader* mShader;
			Transform* mTransform;


			Vector4		mColour;
			AnimationObject* mAnimationObject =nullptr;

			float		mCullSphereRadius;
			float mSqDistToCam;
			bool mOutlined = false;
			bool mIsInstanced = false;

			vector<int>  mMatTextures;

			std::vector<std::vector<Matrix4>> mFrameMatricesVec;


			int		mCurrentFrame;

			int ignoredSubMeshID = -1;
		};
	}
}
