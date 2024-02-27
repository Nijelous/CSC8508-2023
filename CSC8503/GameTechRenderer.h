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

#include "UI.h"




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
			void ClearLights();
			void SetWallFloorObject(GameObject* wallFloorTile) {
				mWallFloorTile = wallFloorTile;
			}

			void SetUIObject(UI* ui) {
				mUi = ui;
			}

		protected:

			enum UBOBlockNames {
				cam,
				MAX
			};
			void InitUBOBlocks();
			void GenUBOBuffers();
			void GenCamMatricesUBOS();
			void FillCamMatricesUBOs();


			void NewRenderLines();
			void NewRenderText();

			void RenderIcons(UI::Icon icon);
			

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
			void LoadDefRendShaders();
			void FillGBuffer(Matrix4& viewMatrix, Matrix4& projMatrix);
			void DrawLightVolumes(Matrix4& viewMatrix, Matrix4& projMatrix);
			void CombineBuffers();
			void DrawOutlinedObjects();
			void LoadSkybox();

			void DrawWallsFloorsInstanced(Matrix4& viewMatrix, Matrix4& projMatrix);

			void SetDebugStringBufferSizes(size_t newVertCount);
			void SetDebugLineBufferSizes(size_t newVertCount);

			void SetUIiconBufferSizes(size_t newVertCount);
			void BindCommonLightDataToShader(OGLShader* shader, Matrix4& viewMatrix, Matrix4& projMatrix);
			void BindSpecificLightDataToShader(Light* l);			
			void SendPointLightDataToShader(OGLShader* shader, PointLight* l);
			void SendSpotLightDataToShader(OGLShader* shader, SpotLight* l);
			void SendDirLightDataToShader(OGLShader* shader, DirectionLight* l);

			vector<const RenderObject*> mActiveObjects;
			vector<const RenderObject*> mOutlinedObjects;

			OGLShader*  debugShader;
			OGLShader*  skyboxShader;
			OGLShader* mOutlineShader;
			OGLShader*  iconShader;

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
			Shader* mPointLightShader;
			Shader* mSpotLightShader;
			Shader* mDirLightShader;
			Shader* mCombineShader;
			const OGLMesh* mSphereMesh;
			OGLMesh* mQuad;
			GameObject* mWallFloorTile;

			vector<Vector3> debugTextPos;
			vector<Vector4> debugTextColours;
			vector<Vector2> debugTextUVs;


			vector<Vector3> UIiconPos;
			vector<Vector2> UIiconUVs;

			//Animation things
			Shader* mShader;
			Mesh* mMesh;
			MeshAnimation* mAnim;
			MeshMaterial* mMaterial;
			vector<GLuint*>  mMatTextures;

			GLuint uBOBlocks[MAX];

			GLuint lineVAO;
			GLuint lineVertVBO;
			size_t lineCount;

			GLuint textVAO;
			GLuint textVertVBO;
			GLuint textColourVBO;
			GLuint textTexVBO;
			size_t textCount;

			GLuint iconVAO;
			GLuint iconVertVBO;
			GLuint iconTexVBO;

			vector<Light*> mLights;

			Frustum mFrameFrustum;

			UI* mUi;
		};
	}
}

