#pragma once
#include "OGLRenderer.h"
#include "OGLShader.h"
#include "OGLTexture.h"
#include "OGLMesh.h"

#include "GameWorld.h"
#include "Frustum.h"

#include "UISystem.h"
#include "WindowsUI.h"


namespace NCL {
	class Maths::Vector3;
	class Maths::Vector4;
	namespace CSC8503 {
		class RenderObject;
        class MiniMap;

		constexpr short MAX_POSSIBLE_LIGHTS = 256;
		constexpr short MAX_POSSIBLE_OBJECTS = 256;

		class GameTechRenderer : public OGLRenderer {
		public:
            friend class MiniMap;

			GameTechRenderer(GameWorld& world);
			~GameTechRenderer();

			Mesh*		LoadMesh(const std::string& name) override;
			void		LoadMeshes(std::unordered_map<std::string, Mesh*>& meshMap, const std::vector<std::string>& details);
			Texture*	LoadTexture(const std::string& name) override;
			GLuint		LoadTextureGetID(const std::string& name);
			Texture* LoadDebugTexture(const std::string& name) override;
			Shader*		LoadShader(const std::string& vertex, const std::string& fragment) override;
			MeshAnimation* LoadAnimation(const std::string& name) override;
			MeshMaterial* LoadMaterial(const std::string& name) override;
			std::vector<int> LoadMeshMaterial(Mesh& mesh, MeshMaterial& meshMaterial);
			

			void AddLight(Light* light);
			void ClearLights();

			void ClearInstanceObjects() { mInstanceTiles.clear(); }

			void SetInstanceObjects(std::unordered_map<std::string, GameObject*>& baseObjects) {
				for (auto const& [key, val] : baseObjects) {
					mInstanceTiles.push_back(val);
				}
			}
			void FillLightUBO();
			void FillTextureDataUBO();
			void SetUIObject(UISystem* ui) {
				mUi = ui;
			}
#ifdef USEGL
            void SetMiniMap(MiniMap* minimap) {
                mMiniMap = minimap;
            }
#endif
			std::function<void()>& GetImguiCanvasFunc();
			void SetImguiCanvasFunc(std::function<void()> func);
		protected:

			enum BufferBlockNames {
				camUBO,
				staticDataUBO,
				lightsUBO,
				objectsUBO,
				animFramesUBO,
				iconUBO,				
				textureDataUBO,
				textureIdUBO,
				cubeTexUBO,
				MAX_UBO
			};

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

			
			struct TextureHandleData {
				GLuint64 handles[256] = { 0 };
			};

			struct TextureHandleIndices {
				int albedoIndex = 0;
				int normalIndex = 0;
				int depthIndex = 0;
				int shadowIndex = 0;
				int albedoLightIndex = 0;
				int specLightIndex = 0;
			};

			void InitUBOBlocks();
			void GenUBOBuffers();
			void GenCamMatricesUBOS();
			void FillCamMatricesUBOs();
			void GenIconUBO();
			void GenStaticDataUBO();
			void GenLightDataUBO();
			void GenObjectDataUBO();
			void GenAnimFramesUBOs();
			void FillObjectDataUBO();	
			void CreateAndFillSkyboxUBO();
			void NewRenderLines();
			void NewRenderText();
			void GenTextureDataUBO();
			void GenTextureIndexUBO();
			void UnbindAllTextures();
			int FindTexHandleIndex(GLuint texId);
			int FindTexHandleIndex(const OGLTexture* tex);

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

			vector<const RenderObject*> mActiveObjects;
			vector<int> mOutlinedObjects;

			vector<std::pair<GLuint, GLuint64>> mTextureHandles;

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

			Frustum mFrameFrustum;

			UISystem* mUi;
			std::unordered_map<std::string, GLuint> mLoadedTextures;
#ifdef USEGL
            MiniMap* mMiniMap{};
#endif
			//TODO(erendgrmnc): added after integrating Imgui lib. Refactor UISystem into this logic.
			std::function<void()> mImguiCanvasFuncToRender = nullptr;
			WindowsUI* mUIHandler;
		};
	}
}

