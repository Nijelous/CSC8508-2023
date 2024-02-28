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

#include "UISystem.h"




namespace NCL {
	class Maths::Vector3;
	class Maths::Vector4;
	namespace CSC8503 {
		class RenderObject;

		class GameTechRenderer : public OGLRenderer	{
		public:

			const short MAX_POSSIBLE_LIGHTS = 100;

			GameTechRenderer(GameWorld& world);
			~GameTechRenderer();

			Mesh*		LoadMesh(const std::string& name);
			Texture*	LoadTexture(const std::string& name);
			Shader*		LoadShader(const std::string& vertex, const std::string& fragment);
			MeshAnimation* LoadAnimation(const std::string& name);
			MeshMaterial* LoadMaterial(const std::string& name);
			

			void AddLight(Light* light);
			void ClearLights();
			void FillLightUBO();
			void SetWallFloorObject(GameObject* wallFloorTile) {
				mWallFloorTile = wallFloorTile;
			}

			void SetUIObject(UISystem* ui) {
				mUi = ui;
			}		

		protected:

			struct LightData {
				//vec3 size, aligned as vec4s
				const float* lightDirection = nullptr;
				const float* lightPos = nullptr;
				const float* lightColour = nullptr;
				//float size
				const float* minDotProd = nullptr;				
				const float* dimDotProd = nullptr;
				const float* lightRadius = nullptr;
							
			};

			enum UBOBlockNames {
				camUBO,
				staticDataUBO,
				lightsUBO,
				MAX_UBO
			};

			void InitUBOBlocks();
			void GenUBOBuffers();
			void GenCamMatricesUBOS();
			void FillCamMatricesUBOs();
			void GenStaticDataUBO();
			void GenLightDataUBO();
			void SendLightDataToGPU(int index, LightData& ld);

			void NewRenderLines();
			void NewRenderText();

			void RenderIcons(UISystem::Icon icon);
			

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
			void FillGBuffer();
			void DrawLightVolumes();
			void CombineBuffers();
			void DrawOutlinedObjects();
			void LoadSkybox();

			void DrawWallsFloorsInstanced();

			void SetDebugStringBufferSizes(size_t newVertCount);
			void SetDebugLineBufferSizes(size_t newVertCount);

			void SetUIiconBufferSizes(size_t newVertCount);
			void BindCommonLightDataToShader(OGLShader* shader);

			vector<const RenderObject*> mActiveObjects;
			vector<const RenderObject*> mOutlinedObjects;

			OGLShader*  mDebugLineShader;
			OGLShader* mDebugTextShader;
			OGLShader*  mSkyboxShader;
			OGLShader* mOutlineShader;
			OGLShader*  mIconShader;

			OGLMesh*	skyboxMesh;
			GLuint		skyboxTex;

			//shadow mapping things
			OGLShader*	mShadowShader;
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

			GLuint uBOBlocks[MAX_UBO];

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

			UISystem* mUi;
		};
	}
}

