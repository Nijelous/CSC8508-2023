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

		constexpr short MAX_INSTANCE_MESHES = 3;
		constexpr short MAX_POSSIBLE_LIGHTS = 256;
		constexpr short MAX_POSSIBLE_OBJECTS = 256;

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

			void ClearInstanceObjects() { mInstanceTiles.clear(); }

			void SetInstanceObjects(GameObject* floorTile, GameObject* wallTile, GameObject* cornerWallTile) {
				mInstanceTiles.push_back(floorTile);
				mInstanceTiles.push_back(wallTile);
				mInstanceTiles.push_back(cornerWallTile);
			}
			void FillLightUBO();
			void FillAnimFrameUBOs(const GameObject& animObj);

			void SetUIObject(UISystem* ui) {
				mUi = ui;
			}		

		protected:

			/* (Author: B Schwarz) Data sent to a UBO buffer can be accessed reliably at offsets of 256 bytes, thus this struct is padded to 256.
			Yes, this means it is 81.25% empty data.
			No, I am not happy about it. */
			struct LightData {
				Vector3 lightDirection = { 0,0,0 };
				float minDotProd = 0.0f;
				Vector3 lightPos = { 0,0,0 };
				float dimDotProd = 0.0f;
				Vector3 lightColour = { 0,0,0 };
				float lightRadius = 0.0f;
				float padding[52] = { 0.0f };
			};

			struct ObjectData {
				Matrix4 modelMatrix;
				Matrix4 shadowMatrix;
				Vector4 objectColour = { 0,0,0,0 };
				bool hasVertexColours = 0;
				float padding[27] = { 0.0f };				
			};

			enum BufferBlockNames {
				camUBO,
				staticDataUBO,
				lightsUBO,
				objectsUBO,
				animFramesUBO,
				MAX_UBO
			};

			void InitUBOBlocks();
			void GenUBOBuffers();
			void GenCamMatricesUBOS();
			void FillCamMatricesUBOs();
			void GenStaticDataUBO();
			void GenLightDataUBO();
			void GenObjectDataUBO();
			void GenAnimFramesUBOs();
			void FillObjectDataUBO();
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
			OGLShader* mAnimatedOutlineShader;
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
			std::vector<GameObject*> mInstanceTiles;

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

