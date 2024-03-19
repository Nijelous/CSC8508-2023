#pragma once
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"

#include "Transform.h"
#include <AnimationObject.h>

#ifdef USEGL
#include <glad/gl.h>

namespace NCL {
	using namespace NCL::Rendering;

	namespace CSC8503 {
		class Transform;
		using namespace Maths;

		class RenderObject
		{
		public:
			RenderObject(Transform* parentTransform, Mesh* mesh, Texture* albedo, Texture* normal, Shader* shader, float cullSphereRadius);
			RenderObject(Transform* parentTransform, Mesh* mesh, Texture* albedo, Texture* normal, Shader* shader, Vector4 colour, float cullSphereRadius);
			~RenderObject();

			void SetAlbedoTexture(Texture* t) {
				mAlbedoTex = t;
			}

			Texture* GetAlbedoTexture() const {
				return mAlbedoTex;
			}

			void SetNormalTexture(Texture* t) {
				mNormalTex = t;
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

			float GetCullSphereRadius() const {
				return mCullSphereRadius;
			}

			float GetSqDistToCam() const {
				return mSqDistToCam;
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

			void SetMatTextures(vector<int> matTextures) {
				mMatTextures = matTextures;
			}

			vector<int>  GetMatTextures() const {
				return mMatTextures;
			}

			void SetFrameMatricesVec(std::vector<std::vector<Matrix4>> frameMatrices) {
				mFrameMatricesVec = frameMatrices;
			}

			std::vector<std::vector<Matrix4>>  GetFrameMatricesVec() const {
				return mFrameMatricesVec;
			}

			AnimationObject* GetAnimationObject() const {
				return mAnimationObject;
			}

			void SetAnimationObject(AnimationObject* newObject) {
				mAnimationObject = newObject;
			}



		protected:
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

		};
	}
}
#endif

#ifdef USEPROSPERO

#include "Buffer.h"

namespace NCL {
	using namespace NCL::Rendering;

	namespace CSC8503 {
		class Transform;
		using namespace Maths;

		class RenderObject
		{
		public:
			RenderObject(Transform* inTransform, Mesh* inMesh, Texture* inTex, Shader* inShader) {
				buffer = nullptr;
				anim = nullptr;

				transform = inTransform;
				mesh = inMesh;
				texture = inTex;
				shader = inShader;
				colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			}
			~RenderObject() {}

			void SetDefaultTexture(Texture* t) {
				texture = t;
			}

			Buffer* GetGPUBuffer() const {
				return buffer;
			}

			void SetGPUBuffer(Buffer* b) {
				buffer = b;
			}

			Texture* GetDefaultTexture() const {
				return texture;
			}

			Mesh* GetMesh() const {
				return mesh;
			}

			Transform* GetTransform() const {
				return transform;
			}

			Shader* GetShader() const {
				return shader;
			}

			void SetColour(const Vector4& c) {
				colour = c;
			}

			Vector4 GetColour() const {
				return colour;
			}

			void SetAnimation(MeshAnimation& inAnim);

			void UpdateAnimation(float dt);

			std::vector<Matrix4>& GetSkeleton() {
				return skeleton;
			}

		protected:
			Buffer* buffer;
			Mesh* mesh;
			Texture* texture;
			Shader* shader;
			Transform* transform;
			Vector4			colour;

			MeshAnimation* anim;

			std::vector<Matrix4> skeleton;
			float	animTime = 0.0f;
			int currentAnimFrame = 0;
		};
	}
}
#endif