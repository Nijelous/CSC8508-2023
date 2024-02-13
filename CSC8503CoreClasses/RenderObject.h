#pragma once
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"

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

			Mesh*	GetMesh() const {
				return mMesh;
			}

			Transform*		GetTransform() const {
				return mTransform;
			}

			Shader*		GetShader() const {
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

		protected:
			Mesh*		mMesh;
			Texture* mAlbedoTex;
			Texture* mNormalTex;
			Shader*		mShader;
			Transform*	mTransform;
			Vector4		mColour;
			float		mCullSphereRadius;
			float mSqDistToCam;
		};
	}
}
