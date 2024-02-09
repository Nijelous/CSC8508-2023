#pragma once
#include "OGLRenderer.h"
#include "OGLShader.h"
#include "OGLTexture.h"
#include "OGLMesh.h"

#include "GameWorld.h"
#include "Frustum.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

#include "MeshAnimation.h"
#include "MeshMaterial.h"



namespace NCL {
	class Maths::Vector3;
	class Maths::Vector4;
	namespace CSC8503 {
		class RenderObject;

		class GameTechRenderer : public OGLRenderer	{
		public:
			GameTechRenderer(GameWorld& world);
			~GameTechRenderer();

			Mesh*		LoadMesh(const std::string& name);
			Texture*	LoadTexture(const std::string& name);
			Shader*		LoadShader(const std::string& vertex, const std::string& fragment);
			MeshAnimation* LoadAnimation(const std::string& name);
			MeshMaterial* LoadMaterial(const std::string& name);
			

			void AddLight(Light* light);

		protected:
			void NewRenderLines();
			void NewRenderText();
			

			void RenderFrame()	override;

			OGLShader*		defaultShader;

			GameWorld&	gameWorld;

			void BuildObjectList();
			void SortObjectList();
			void RenderShadowMap();
			void RenderCamera(); 
			void RenderSkybox();
			void SetUpFBOs();
			void GenerateScreenTexture(GLuint &fbo, bool depth = false);
			void BindTexAttachmentsToBuffers(GLuint& fbo, GLuint& colourAttach0, GLuint& colourAttach1, GLuint* depthTex = nullptr);
			void MakeAndAttachStencil(GLuint& fbo, GLuint& stencil);
			void LoadDefRendShaders();
			void FillGBuffer(Matrix4& viewMatrix, Matrix4& projMatrix);
			void DrawLightVolumes(Matrix4& viewMatrix, Matrix4& projMatrix);
			void CombineBuffers();
			
			void LoadSkybox();


			void SetDebugStringBufferSizes(size_t newVertCount);
			void SetDebugLineBufferSizes(size_t newVertCount);

			void SendLightDataToShader(Light* l, OGLShader* shader);
			void SendPointLightDataToShader(OGLShader* shader, PointLight* l);
			void SendSpotLightDataToShader(OGLShader* shader, SpotLight* l);
			void SendDirLightDataToShader(OGLShader* shader, DirectionLight* l);

			vector<const RenderObject*> activeObjects;

			OGLShader*  debugShader;
			OGLShader*  skyboxShader;
			OGLMesh*	skyboxMesh;
			GLuint		skyboxTex;

			//shadow mapping things
			OGLShader*	shadowShader;
			GLuint		shadowTex;
			GLuint		shadowFBO;
			Matrix4     shadowMatrix;

			//Debug data storage things
			vector<Vector3> debugLineData;

			//Deferred rendering stuff
			GLuint mGBufferFBO;
			GLuint mGBufferColourTex;
			GLuint mGBufferNormalTex;
			GLuint mGBufferDepthTex;
			GLuint mLightFBO;
			GLuint mLightAlbedoTex;
			GLuint mLightSpecularTex;
			Shader* mLightShader;
			Shader* mCombineShader;
			const OGLMesh* mSphereMesh;
			OGLMesh* mQuad;

			vector<Vector3> debugTextPos;
			vector<Vector4> debugTextColours;
			vector<Vector2> debugTextUVs;

			GLuint lineVAO;
			GLuint lineVertVBO;
			size_t lineCount;

			GLuint textVAO;
			GLuint textVertVBO;
			GLuint textColourVBO;
			GLuint textTexVBO;
			size_t textCount;
			vector<Light*> mLights;

			Frustum mFrameFrustum;
		};
	}
}

