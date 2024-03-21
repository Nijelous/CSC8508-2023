#include "imgui/imgui_impl_win32.h"
#ifdef USEGL

#include "GameTechRenderer.h"
#include "GameObject.h"
#include "RenderObject.h"
#include "Camera.h"
#include "TextureLoader.h"
#include "MshLoader.h"
#include "UISystem.h"
#include "Mesh.h"
#include "Debug.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#ifdef USEGL
#include "MiniMap.h"
#endif

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

#define SHADOWSIZE 4096

Matrix4 biasMatrix = Matrix4::Translation(Vector3(0.5f, 0.5f, 0.5f)) * Matrix4::Scale(Vector3(0.5f, 0.5f, 0.5f));

GameTechRenderer::GameTechRenderer(GameWorld& world) : OGLRenderer(*Window::GetWindow()), gameWorld(world) {
	glEnable(GL_DEPTH_TEST);

	mDebugLineShader = new OGLShader("DebugLines.vert", "DebugLines.frag");
	mDebugTextShader = new OGLShader("DebugText.vert", "DebugText.frag");
	mShadowShader = new OGLShader("shadow.vert", "shadow.frag");
	mOutlineShader = new OGLShader("outline.vert", "outline.frag");
	mAnimatedOutlineShader = new OGLShader("outlineAnimated.vert", "outline.frag");
	mIconShader = new OGLShader("UI.vert", "UI.frag");

	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(1, 1, 1, 1);

	//Skybox!
	mSkyboxShader = new OGLShader("skybox.vert", "skybox.frag");
	skyboxMesh = new OGLMesh();
	skyboxMesh->SetVertexPositions({ Vector3(-1, 1,-1), Vector3(-1,-1,-1) , Vector3(1,-1,-1) , Vector3(1,1,-1) });
	skyboxMesh->SetVertexIndices({ 0,1,2,2,3,0 });
	skyboxMesh->UploadToGPU();

	//set up a quad
	mQuad = new OGLMesh();
	mQuad->SetPrimitiveType(GeometryPrimitive::TriangleStrip);
	mQuad->SetVertexPositions({ Vector3(-1, 1,0), Vector3(-1,-1,0) , Vector3(1,1,0) , Vector3(1,-1,0) });
	mQuad->SetVertexTextureCoords({ Vector2(0.0f, 1.0f), Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), Vector2(1.0f, 0.0f) });

	mQuad->UploadToGPU();

	LoadSkybox();
	SetUpFBOs();
	LoadDefRendShaders();
	InitUBOBlocks();
	GenUBOBuffers();
	mSphereMesh = (OGLMesh*)LoadMesh("sphere.msh");

	glGenVertexArrays(1, &lineVAO);
	glGenVertexArrays(1, &textVAO);
	glGenVertexArrays(1, &iconVAO);

	glGenBuffers(1, &lineVertVBO);
	glGenBuffers(1, &textVertVBO);
	glGenBuffers(1, &textColourVBO);
	glGenBuffers(1, &textTexVBO);

	glGenBuffers(1, &iconVertVBO);
	glGenBuffers(1, &iconTexVBO);

	Debug::CreateDebugFont("PressStart2P.fnt", *LoadDebugTexture("PressStart2P.png"));

	SetDebugStringBufferSizes(10000);
	SetDebugLineBufferSizes(1000);
	FillTextureDataUBO();
	mUIHandler = new WindowsUI();
}

GameTechRenderer::~GameTechRenderer() {
	UnbindAllTextures();
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);

	glDeleteFramebuffers(1, &mGBufferFBO);
	glDeleteFramebuffers(1, &mLightFBO);
	glDeleteTextures(1, &mGBufferColourTex);
	glDeleteTextures(1, &mGBufferNormalTex);
	glDeleteTextures(1, &mGBufferDepthTex);
	glDeleteTextures(1, &mLightAlbedoTex);
	glDeleteTextures(1, &mLightSpecularTex);

	delete mPointLightShader;
	delete mSpotLightShader;
	delete mDirLightShader;
	delete mCombineShader;
	delete mOutlineShader;
	delete mAnimatedOutlineShader;
	delete mDebugLineShader;
	delete mDebugTextShader;
	delete mSkyboxShader;
	delete mIconShader;
	delete mUIHandler;
	ClearLights();
}

void GameTechRenderer::LoadSkybox() {
	std::string filenames[6] = {
		"/Cubemap/FS002_Night_Cubemap_right.png",
		"/Cubemap/FS002_Night_Cubemap_left.png",
		"/Cubemap/FS002_Night_Cubemap_up.png",
		"/Cubemap/FS002_Night_Cubemap_down.png",
		"/Cubemap/FS002_Night_Cubemap_back.png",
		"/Cubemap/FS002_Night_Cubemap_front.png"
	};

	int width[6] = { 0 };
	int height[6] = { 0 };
	int channels[6] = { 0 };
	int flags[6] = { 0 };

	vector<char*> texData(6, nullptr);

	for (int i = 0; i < 6; ++i) {
		TextureLoader::LoadTexture(filenames[i], texData[i], width[i], height[i], channels[i], flags[i]);
		if (i > 0 && (width[i] != width[0] || height[0] != height[0])) {
			std::cout << __FUNCTION__ << " cubemap input textures don't match in size?\n";
			return;
		}
	}
	glGenTextures(1, &skyboxTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	GLenum type = channels[0] == 4 ? GL_RGBA : GL_RGB;

	for (int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width[i], height[i], 0, type, GL_UNSIGNED_BYTE, texData[i]);
	}

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	CreateAndFillSkyboxUBO();
	
}

void GameTechRenderer::CreateAndFillSkyboxUBO() {
	glGenBuffers(1, &uBOBlocks[cubeTexUBO]);
	glBindBufferRange(GL_UNIFORM_BUFFER, cubeTexUBO, uBOBlocks[cubeTexUBO], 0, sizeof(GLuint64));
	const GLuint64 handle = glGetTextureHandleARB(skyboxTex);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(TextureHandleData), &handle, GL_STATIC_DRAW);
	glMakeTextureHandleResidentARB(handle);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GameTechRenderer::LoadDefRendShaders() {
	mPointLightShader = LoadShader("light.vert", "pointlight.frag");
	mSpotLightShader = LoadShader("light.vert", "spotlight.frag");
	mDirLightShader = LoadShader("light.vert", "dirlight.frag");
	mCombineShader = LoadShader("combine.vert", "combine.frag");
}

void GameTechRenderer::InitUBOBlocks() {
	for (int i = 0; i < MAX_UBO; i++) {
		uBOBlocks[i] = 100 + i;
	}
}

void GameTechRenderer::GenUBOBuffers() {
	GenCamMatricesUBOS();
	GenStaticDataUBO();
	GenLightDataUBO();
	GenObjectDataUBO();
	GenAnimFramesUBOs();
	GenIconUBO();
	GenTextureDataUBO();
	GenTextureIndexUBO();
}

void GameTechRenderer::GenIconUBO() {
	glGenBuffers(1, &uBOBlocks[iconUBO]);
	glBindBuffer(GL_UNIFORM_BUFFER, uBOBlocks[iconUBO]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GameTechRenderer::GenCamMatricesUBOS() {
	glGenBuffers(1, &uBOBlocks[camUBO]);
	glBindBuffer(GL_UNIFORM_BUFFER, uBOBlocks[camUBO]);
	glBufferData(GL_UNIFORM_BUFFER, 51 * sizeof(float), NULL, GL_STATIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, camUBO, uBOBlocks[camUBO], 0, 51 * sizeof(float));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GameTechRenderer::FillCamMatricesUBOs() {
	Matrix4 viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());
	Matrix4 invProjView = (projMatrix * viewMatrix).Inverse();

	Vector3 cameraPos = gameWorld.GetMainCamera().GetPosition();
	mFrameFrustum = mFrameFrustum.FromViewProjMatrix(projMatrix * viewMatrix);
	glBindBuffer(GL_UNIFORM_BUFFER, uBOBlocks[camUBO]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, 16 * sizeof(float), (float*)&projMatrix);
	glBufferSubData(GL_UNIFORM_BUFFER, 16 * sizeof(float), 16 * sizeof(float), (float*)&viewMatrix);
	glBufferSubData(GL_UNIFORM_BUFFER, 32 * sizeof(float), 16 * sizeof(float), (float*)&invProjView);
	glBufferSubData(GL_UNIFORM_BUFFER, 48 * sizeof(float), 3 * sizeof(float), (float*)&cameraPos);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GameTechRenderer::GenStaticDataUBO() {
	Matrix4 orthProj = Matrix4::Orthographic(0.0, 100.0f, 100, 0, -1.0f, 1.0f);
	Vector2 pixelSize(1.0f / hostWindow.GetScreenSize().x, 1.0f / hostWindow.GetScreenSize().y);
	glGenBuffers(1, &uBOBlocks[staticDataUBO]);
	glBindBuffer(GL_UNIFORM_BUFFER, uBOBlocks[staticDataUBO]);
	glBufferData(GL_UNIFORM_BUFFER, 18 * sizeof(float), NULL, GL_STATIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, staticDataUBO, uBOBlocks[staticDataUBO], 0, 18 * sizeof(float));
	glBufferSubData(GL_UNIFORM_BUFFER, 0, 16 * sizeof(float), (float*)&orthProj);
	glBufferSubData(GL_UNIFORM_BUFFER, 16 * sizeof(float), 2 * sizeof(float), (float*)&pixelSize);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GameTechRenderer::GenObjectDataUBO() {
	glGenBuffers(1, &uBOBlocks[objectsUBO]);
	glBindBuffer(GL_UNIFORM_BUFFER, uBOBlocks[objectsUBO]);
	glBufferData(GL_UNIFORM_BUFFER, MAX_POSSIBLE_OBJECTS * sizeof(ObjectData), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GameTechRenderer::GenLightDataUBO() {
	glGenBuffers(1, &uBOBlocks[lightsUBO]);
	glBindBuffer(GL_UNIFORM_BUFFER, uBOBlocks[lightsUBO]);
	glBufferData(GL_UNIFORM_BUFFER, MAX_POSSIBLE_LIGHTS * sizeof(LightData), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GameTechRenderer::GenAnimFramesUBOs() {
	glGenBuffers(1, &uBOBlocks[animFramesUBO]);
	glBindBuffer(GL_UNIFORM_BUFFER, uBOBlocks[animFramesUBO]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrix4) * 128, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GameTechRenderer::GenTextureDataUBO() {
	glGenBuffers(1, &uBOBlocks[textureDataUBO]);
	glBindBuffer(GL_UNIFORM_BUFFER, uBOBlocks[textureDataUBO]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(TextureHandleData), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GameTechRenderer::GenTextureIndexUBO() {
	glGenBuffers(1, &uBOBlocks[textureIdUBO]);
	glBindBuffer(GL_UNIFORM_BUFFER, uBOBlocks[textureIdUBO]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(TextureHandleIndices), NULL, GL_DYNAMIC_DRAW);	
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GameTechRenderer::FillTextureDataUBO() {
	glBindBufferRange(GL_UNIFORM_BUFFER, textureDataUBO, uBOBlocks[textureDataUBO], 0, sizeof(float));
	TextureHandleData handlesToUpload;
	
	for (int i = 0; i < mTextureHandles.size(); i++) {
		handlesToUpload.handles[i * 2] = mTextureHandles[i].second;
	}
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(TextureHandleData), &handlesToUpload);
}

std::function<void()>& GameTechRenderer::GetImguiCanvasFunc() {
	return mImguiCanvasFuncToRender;
}

void GameTechRenderer::SetImguiCanvasFunc(std::function<void()> func) {
	mImguiCanvasFuncToRender = func;
}


void GameTechRenderer::UnbindAllTextures() {
	for (std::pair<GLuint, GLuint64> handle : mTextureHandles) {
		glMakeTextureHandleNonResidentARB(handle.second);
	}
}

void GameTechRenderer::FillLightUBO() {
	glBindBuffer(GL_UNIFORM_BUFFER, uBOBlocks[lightsUBO]);
	glBindBufferRange(GL_UNIFORM_BUFFER, lightsUBO, uBOBlocks[lightsUBO], 0, MAX_POSSIBLE_LIGHTS * sizeof(LightData));
	LightData* lightData = new LightData[MAX_POSSIBLE_LIGHTS];
	for (int i = 0; i < mLights.size(); i++) {

		LightData ld;
		Light::Type type = mLights[i]->GetType();
		DirectionLight* dLight = (DirectionLight*)mLights[i];
		PointLight* pLight = (PointLight*)mLights[i];
		SpotLight* sLight = (SpotLight*)mLights[i];

		switch (type) {
		case Light::Direction:
			ld.lightDirection = dLight->GetDirection();
			ld.lightRadius = dLight->GetRadius();
			ld.lightPos = dLight->GetCentre();
			ld.lightColour = dLight->GetColour();
			lightData[i] = ld;
			break;

		case Light::Type::Point:
			ld.lightRadius = pLight->GetRadius();
			ld.lightPos = pLight->GetPosition();
			ld.lightColour = pLight->GetColour();
			lightData[i] = ld;
			break;

		case Light::Type::Spot:
			ld.lightDirection = sLight->GetDirection();
			ld.lightRadius = sLight->GetRadius();
			ld.lightPos = sLight->GetPosition();
			ld.minDotProd = sLight->GetDimProdMin();
			ld.lightColour = sLight->GetColour();
			ld.dimDotProd = sLight->GetDimProdMin();
			lightData[i] = ld;
			break;

		default:
			break;
		}
	}
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lightData[0]) * mLights.size(), &lightData[0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	delete[] lightData;
}

void GameTechRenderer::FillObjectDataUBO() {
	if (mActiveObjects.empty()) return;
	glBindBuffer(GL_UNIFORM_BUFFER, uBOBlocks[objectsUBO]);
	ObjectData* objectData = new ObjectData[MAX_POSSIBLE_OBJECTS];

	for (int i = 0; i < mActiveObjects.size(); i++) {
		ObjectData od;
		od.modelMatrix = mActiveObjects[i]->GetTransform()->GetMatrix();
		od.shadowMatrix = shadowMatrix * od.modelMatrix;
		od.objectColour = mActiveObjects[i]->GetColour();
		od.hasVertexColours = mActiveObjects[i]->GetMesh()->GetColourData().empty() ? 0 : 1;
		objectData[i] = od;
	}
	glBufferData(GL_UNIFORM_BUFFER,
		sizeof(ObjectData) * mActiveObjects.size(), &objectData[0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	delete[] objectData;
}

void GameTechRenderer::RenderFrame() {

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glClearColor(1, 1, 1, 1);
	BuildObjectList();
	SortObjectList();
	FillObjectDataUBO();
	FillCamMatricesUBOs();
	RenderCamera();
	RenderSkybox();
	DrawOutlinedObjects();
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	NewRenderLines();
	NewRenderText();
	if (mIsGameStarted) {
		const std::vector<UISystem::Icon*>& icons = mUi->GetIcons();
		if (mUi) {
			for (auto& i : icons) {
				RenderIcons(*i);
			}
		}
	}
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mUIHandler->RenderUI(mImguiCanvasFuncToRender);

#ifdef USEGL
    if (mMiniMap)
        mMiniMap->Render();
#endif
}

void GameTechRenderer::BuildObjectList() {
	mActiveObjects.clear();
	mOutlinedObjects.clear();
	int x;
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &x);

	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			if (o->IsRendered()) {
				RenderObject* rendObj = o->GetRenderObject();
				bool isInFrustum = mFrameFrustum.SphereInsideFrustum(o->GetTransform().GetPosition(), o->GetRenderObject()->GetCullSphereRadius());
				if (rendObj && isInFrustum && !rendObj->IsInstanced()) {
					rendObj->SetSqDistToCam(gameWorld.GetMainCamera().GetPosition());
					mActiveObjects.emplace_back(rendObj);
				}
			}
		}
	);
}

void GameTechRenderer::SortObjectList() {
	std::sort(mActiveObjects.begin(), mActiveObjects.end(), RenderObject::CompareBySqCamDist);
	for (int i = 0; i < mActiveObjects.size(); i++) {
		if (mActiveObjects[i]->GetOutlined()) {
			mOutlinedObjects.emplace_back(i);
		}
	}
}

void GameTechRenderer::RenderShadowMap() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);

	glCullFace(GL_FRONT);

	BindShader(*mShadowShader);
	int mvpLocation = glGetUniformLocation(mShadowShader->GetProgramID(), "mvpMatrix");
	PointLight* pl = (PointLight*)mLights[0];
	Matrix4 shadowViewMatrix = Matrix4::BuildViewMatrix(pl->GetPosition(), Vector3(0, 0, 0), Vector3(0, 1, 0));
	Matrix4 shadowProjMatrix = Matrix4::Perspective(100.0f, 500.0f, 1, 45.0f);

	Matrix4 mvMatrix = shadowProjMatrix * shadowViewMatrix;

	shadowMatrix = biasMatrix * mvMatrix; //we'll use this one later on

	for (const auto& i : mActiveObjects) {
		Matrix4 modelMatrix = (*i).GetTransform()->GetMatrix();
		Matrix4 mvpMatrix = mvMatrix * modelMatrix;
		glUniformMatrix4fv(mvpLocation, 1, false, (float*)&mvpMatrix);
		BindMesh((OGLMesh&)*(*i).GetMesh());
		size_t layerCount = (*i).GetMesh()->GetSubMeshCount();
		for (size_t i = 0; i < layerCount; ++i) {
			DrawBoundMesh((uint32_t)i);
		}
	}

	glViewport(0, 0, windowSize.x, windowSize.y);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCullFace(GL_BACK);
}

void GameTechRenderer::RenderSkybox() {
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mGBufferFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	int width = hostWindow.GetScreenSize().x;
	int height = hostWindow.GetScreenSize().y;
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	BindShader(*mSkyboxShader);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	BindMesh(*skyboxMesh);
	DrawBoundMesh();
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void GameTechRenderer::RenderCamera() {
	FillGBuffer();
	DrawLightVolumes();
	CombineBuffers();
}

void GameTechRenderer::DrawWallsFloorsInstanced() {
	for (int i = 0; i < mInstanceTiles.size(); i++) {
		RenderObject* rendObj = mInstanceTiles[i]->GetRenderObject();
		OGLShader* shader = (OGLShader*)rendObj->GetShader();
		BindShader(*shader);
		TextureHandleIndices texInds;
		OGLMesh* mesh = (OGLMesh*)rendObj->GetMesh();
		size_t layerCount = mesh->GetSubMeshCount();
		BindMesh(*mesh);
		if (rendObj->GetMatTextures().size() > 1) {
			for (size_t b = 0; b < layerCount; ++b) {
				if(rendObj->GetMatTextures()[b * 2] == 0) texInds.albedoIndex = FindTexHandleIndex((OGLTexture*)rendObj->GetAlbedoTexture());
				texInds.albedoIndex = FindTexHandleIndex(rendObj->GetMatTextures()[b * 2]);
				if(rendObj->GetMatTextures()[(b * 2) + 1] == 0) texInds.normalIndex = FindTexHandleIndex((OGLTexture*)rendObj->GetNormalTexture());
				else texInds.normalIndex = FindTexHandleIndex(rendObj->GetMatTextures()[(b * 2) + 1]);
				glBindBufferBase(GL_UNIFORM_BUFFER, textureIdUBO, uBOBlocks[textureIdUBO]);
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(texInds), &texInds);
				DrawBoundMesh((uint32_t)b, mesh->GetInstanceMatricesSize());
			}
		}
		else {
			texInds.albedoIndex = FindTexHandleIndex((OGLTexture*)rendObj->GetAlbedoTexture());
			texInds.normalIndex = FindTexHandleIndex((OGLTexture*)rendObj->GetNormalTexture());
			glBindBufferBase(GL_UNIFORM_BUFFER, textureIdUBO, uBOBlocks[textureIdUBO]);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(texInds), &texInds);
			for (size_t b = 0; b < layerCount; ++b) {
				DrawBoundMesh((uint32_t)b, mesh->GetInstanceMatricesSize());
			}
		}
	}
}

void GameTechRenderer::FillGBuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, mGBufferFBO);
	glEnable(GL_STENCIL_TEST);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	if (!mInstanceTiles.empty()) DrawWallsFloorsInstanced();
	OGLShader* activeShader = nullptr;
	OGLMesh* activeMesh = nullptr;

	for (int i = 0; i < mActiveObjects.size(); i++) {
		OGLShader* shader = (OGLShader*)mActiveObjects[i]->GetShader();
		if (activeShader != shader) {
			activeShader = shader;
			BindShader(*shader);
		}		

		OGLMesh* mesh = (OGLMesh*) mActiveObjects[i]->GetMesh();
		if (activeMesh != mesh) {
			BindMesh(*mesh);
			activeMesh = mesh;
		}
		size_t layerCount = mesh->GetSubMeshCount();
		TextureHandleIndices texInds;		

		glBindBufferRange(GL_UNIFORM_BUFFER, objectsUBO, uBOBlocks[objectsUBO], i * sizeof(ObjectData), sizeof(float));
		//Animation basic draw
		if (mActiveObjects[i]->GetAnimationObject()) {
			for (size_t b = 0; b < layerCount; ++b) {				
				vector<Matrix4> frameMatrices = mActiveObjects[i]->GetFrameMatricesVec()[b];
				Matrix4* frameData = new Matrix4[128];
				glBindBufferBase(GL_UNIFORM_BUFFER, animFramesUBO, uBOBlocks[animFramesUBO]);
				for (int i = 0; i < frameMatrices.size(); i++) frameData[i] = frameMatrices[i];
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrix4) * 128, frameData);
				delete[] frameData;
				texInds.albedoIndex = FindTexHandleIndex(mActiveObjects[i]->GetMatTextures()[b * 2]);
				if (mActiveObjects[i]->GetMatTextures()[(b * 2) + 1] == 0) texInds.normalIndex = FindTexHandleIndex((OGLTexture*)mActiveObjects[i]->GetNormalTexture());
				else texInds.normalIndex = FindTexHandleIndex(mActiveObjects[i]->GetMatTextures()[(b * 2) + 1]);
				glBindBufferBase(GL_UNIFORM_BUFFER, textureIdUBO, uBOBlocks[textureIdUBO]);
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(texInds), &texInds);
				DrawBoundMesh((uint32_t)b);
			}
		}
		else {
			if (mActiveObjects[i]->GetMatTextures().size() > 1) {
				for (size_t b = 0; b < layerCount; ++b) {
					texInds.albedoIndex = FindTexHandleIndex(mActiveObjects[i]->GetMatTextures()[b]);
					if (mActiveObjects[i]->GetMatTextures()[(b * 2) + 1] == 0) texInds.normalIndex = FindTexHandleIndex((OGLTexture*)mActiveObjects[i]->GetNormalTexture());
					else texInds.normalIndex = FindTexHandleIndex(mActiveObjects[i]->GetMatTextures()[(b * 2) + 1]);
					glBindBufferBase(GL_UNIFORM_BUFFER, textureIdUBO, uBOBlocks[textureIdUBO]);
					glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(texInds), &texInds);
					DrawBoundMesh((uint32_t)b, mesh->GetInstanceMatricesSize());
				}
			}
			else {
				texInds.albedoIndex = FindTexHandleIndex((OGLTexture*)mActiveObjects[i]->GetAlbedoTexture());
				texInds.normalIndex = FindTexHandleIndex((OGLTexture*)mActiveObjects[i]->GetNormalTexture());
				glBindBufferBase(GL_UNIFORM_BUFFER, textureIdUBO, uBOBlocks[textureIdUBO]);
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(texInds), &texInds);
				for (size_t b = 0; b < layerCount; ++b) {
					DrawBoundMesh((uint32_t)b);
				}
			}
		}
	}
	glDisable(GL_STENCIL_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GameTechRenderer::DrawLightVolumes() {
	glBindFramebuffer(GL_FRAMEBUFFER, mLightFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);
	TextureHandleIndices texInds;
	texInds.normalIndex = FindTexHandleIndex(mGBufferNormalTex);
	texInds.depthIndex = FindTexHandleIndex(mGBufferDepthTex);
	glBindBufferBase(GL_UNIFORM_BUFFER, textureIdUBO, uBOBlocks[textureIdUBO]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(texInds), &texInds);
	OGLShader* activeShader = nullptr;
	BindMesh(*mSphereMesh);
	for (int i = 0; i < mLights.size(); i++) {
		OGLShader* shader = nullptr;
		glBindBufferRange(GL_UNIFORM_BUFFER, lightsUBO, uBOBlocks[lightsUBO], i * sizeof(LightData), sizeof(float));
		if (mLights[i]->GetType() == Light::Point) shader = (OGLShader*) mPointLightShader;
		if (mLights[i]->GetType() == Light::Direction) shader = (OGLShader*) mDirLightShader;
		if (mLights[i]->GetType() == Light::Spot) shader = (OGLShader*)mSpotLightShader;
		if (shader != activeShader) {
			BindShader(*shader);
			activeShader = shader;
		}		
		DrawBoundMesh();
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);	
	glClearColor(0.2f, 0.2f, 0.2f, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GameTechRenderer::CombineBuffers() {
	OGLShader* shader = (OGLShader*)mCombineShader;
	BindShader(*shader);
	TextureHandleIndices texInds;
	texInds.albedoIndex = FindTexHandleIndex(mGBufferColourTex);
	texInds.albedoLightIndex = FindTexHandleIndex(mLightAlbedoTex);
	texInds.specLightIndex = FindTexHandleIndex(mLightSpecularTex);
	glBindBufferBase(GL_UNIFORM_BUFFER, textureIdUBO, uBOBlocks[textureIdUBO]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(texInds), &texInds);
	BindMesh(*mQuad);
	DrawBoundMesh();
}

void GameTechRenderer::DrawOutlinedObjects() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glClear(GL_DEPTH_BUFFER_BIT);
	TextureHandleIndices texInds;
	texInds.depthIndex = FindTexHandleIndex(mGBufferDepthTex);
	glBindBufferBase(GL_UNIFORM_BUFFER, textureIdUBO, uBOBlocks[textureIdUBO]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(texInds), &texInds);

	for (int ind : mOutlinedObjects) {
		OGLShader* shader = (mActiveObjects[ind]->GetAnimationObject()) ? mAnimatedOutlineShader : mOutlineShader;
		BindShader(*shader);
		OGLMesh* mesh = (OGLMesh*)mActiveObjects[ind]->GetMesh();
		BindMesh(*mesh);
		size_t layerCount = mesh->GetSubMeshCount();
		glBindBufferRange(GL_UNIFORM_BUFFER, objectsUBO, uBOBlocks[objectsUBO],	ind * sizeof(ObjectData), sizeof(float));
		for (size_t b = 0; b < layerCount; ++b) {
			if (mActiveObjects[ind]->GetAnimationObject()) {
				vector<Matrix4> frameMatrices = mActiveObjects[ind]->GetFrameMatricesVec()[b];
				Matrix4* frameData = new Matrix4[128];
				glBindBufferBase(GL_UNIFORM_BUFFER, animFramesUBO, uBOBlocks[animFramesUBO]);
				for (int i = 0; i < frameMatrices.size(); i++) frameData[i] = frameMatrices[i];
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrix4) * 128, frameData);
			}
			DrawBoundMesh((uint32_t)b);
		}
	}
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void GameTechRenderer::SetUpFBOs() {
	glGenFramebuffers(1, &mGBufferFBO);
	glGenFramebuffers(1, &mLightFBO);

	GenerateScreenTexture(mGBufferColourTex);
	GenerateScreenTexture(mGBufferNormalTex);
	GenerateScreenTexture(mGBufferDepthTex, true);
	GenerateScreenTexture(mLightAlbedoTex);
	GenerateScreenTexture(mLightSpecularTex);

	BindTexAttachmentsToBuffers(mGBufferFBO, mGBufferColourTex, mGBufferNormalTex, &mGBufferDepthTex);
	BindTexAttachmentsToBuffers(mLightFBO, mLightAlbedoTex, mLightSpecularTex);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
}

void GameTechRenderer::GenerateScreenTexture(GLuint& tex, bool depth) {
	glCreateTextures(GL_TEXTURE_2D, 1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint internalFormat = depth ? GL_DEPTH24_STENCIL8 : GL_RGBA8;
	GLuint format = depth ? GL_DEPTH_STENCIL : GL_RGBA;
	GLuint type = depth ? GL_UNSIGNED_INT_24_8 : GL_UNSIGNED_BYTE;
	

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, hostWindow.GetScreenSize().x, hostWindow.GetScreenSize().y, 0, format, type, NULL);

	if (FindTexHandleIndex(tex) == -1) {
		const GLuint64 handle = glGetTextureHandleARB(tex);
		glMakeTextureHandleResidentARB(handle);
		mTextureHandles.push_back(std::pair<GLuint, GLuint64>(tex, handle));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}


void GameTechRenderer::BindTexAttachmentsToBuffers(GLuint& fbo, GLuint& colour0, GLuint& colour1, GLuint* depthTex) {
	GLenum buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colour0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, colour1, 0);
	if (depthTex) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, *depthTex, 0);
	}
	glDrawBuffers(2, buffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return;
}


Mesh* GameTechRenderer::LoadMesh(const std::string& name) {
	OGLMesh* mesh = new OGLMesh();
	MshLoader::LoadMesh(name, *mesh);
	mesh->SetPrimitiveType(GeometryPrimitive::Triangles);
	mesh->UploadToGPU();
	return mesh;
}

void GameTechRenderer::LoadMeshes(std::unordered_map<std::string, Mesh*>& meshMap, const std::vector<std::string>& details) {
	std::vector<OGLMesh*> meshes;
	for (int i = 0; i < details.size(); i += 3) {
		meshes.push_back(new OGLMesh());
	}
	std::thread fileLoadThreads[4];
	int loadSplit = details.size() / 12;
	for (int i = 0; i < 4; i++) {
		fileLoadThreads[i] = std::thread([meshes, details, i, loadSplit] {
			int endPoint = i == 3 ? details.size() / 3 : loadSplit * (i + 1);
			for (int j = loadSplit * i; j < endPoint; j++) {
				MshLoader::LoadMesh(details[(j * 3) + 1], *meshes[j]);
				meshes[j]->SetPrimitiveType(GeometryPrimitive::Triangles);
			}
			});
	}
	for (int i = 0; i < 4; i++) {
		fileLoadThreads[i].join();
	}
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i]->UploadToGPU();
		meshMap[details[i*3]] = meshes[i];
	}
}

void GameTechRenderer::NewRenderLines() {
	const std::vector<Debug::DebugLineEntry>& lines = Debug::GetDebugLines();
	if (lines.empty()) {
		return;
	}
	BindShader(*mDebugLineShader);

	debugLineData.clear();

	size_t frameLineCount = lines.size() * 2;

	SetDebugLineBufferSizes(frameLineCount);

	glBindBuffer(GL_ARRAY_BUFFER, lineVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, lines.size() * sizeof(Debug::DebugLineEntry), lines.data());


	glBindVertexArray(lineVAO);
	glDrawArrays(GL_LINES, 0, (GLsizei)frameLineCount);
	glBindVertexArray(0);
}

void GameTechRenderer::RenderIcons(UISystem::Icon i) {

	BindShader(*mIconShader);

	int iconVertCount = 6;

	UIiconPos.clear();
	UIiconUVs.clear();

	TextureHandleIndices texInds;
	texInds.albedoIndex = FindTexHandleIndex((OGLTexture*)i.mTexture);
	glBindBufferBase(GL_UNIFORM_BUFFER, textureIdUBO, uBOBlocks[textureIdUBO]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(texInds), &texInds);

	mUi->BuildVerticesForIcon(i.mPosition, i.mLength, i.mHeight, UIiconPos, UIiconUVs);

	glBindBufferBase(GL_UNIFORM_BUFFER, iconUBO, uBOBlocks[iconUBO]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float), &i.mTransparency);

	Matrix4 proj = Matrix4::Orthographic(0.0, 100.0f, 100, 0, -1.0f, 1.0f);
	SetUIiconBufferSizes(iconVertCount);

	glBindBuffer(GL_ARRAY_BUFFER, iconVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, iconVertCount * sizeof(Vector3), UIiconPos.data());
	glBindBuffer(GL_ARRAY_BUFFER, iconTexVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, iconVertCount * sizeof(Vector2), UIiconUVs.data());

	glEnable(GL_BLEND);

	glBindVertexArray(iconVAO);
	glDrawArrays(GL_TRIANGLES, 0, iconVertCount);
	glBindVertexArray(0);
}

void GameTechRenderer::NewRenderText() {
	const std::vector<Debug::DebugStringEntry>& strings = Debug::GetDebugStrings();
	if (strings.empty()) {
		return;
	}

	BindShader(*mDebugTextShader);

	OGLTexture* t = (OGLTexture*)Debug::GetDebugFont()->GetTexture();

	TextureHandleIndices texInds;
	texInds.albedoIndex = FindTexHandleIndex(t);
	glBindBufferBase(GL_UNIFORM_BUFFER, textureIdUBO, uBOBlocks[textureIdUBO]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(texInds), &texInds);

	debugTextPos.clear();
	debugTextColours.clear();
	debugTextUVs.clear();

	int frameVertCount = 0;
	for (const auto& s : strings) {
		frameVertCount += Debug::GetDebugFont()->GetVertexCountForString(s.data);
	}
	SetDebugStringBufferSizes(frameVertCount);

	for (const auto& s : strings) {
		float size = s.fontSize;
		Debug::GetDebugFont()->BuildVerticesForString(s.data, s.position, s.colour, size, debugTextPos, debugTextUVs, debugTextColours);
	}

	glBindBuffer(GL_ARRAY_BUFFER, textVertVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector3), debugTextPos.data());
	glBindBuffer(GL_ARRAY_BUFFER, textColourVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector4), debugTextColours.data());
	glBindBuffer(GL_ARRAY_BUFFER, textTexVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, frameVertCount * sizeof(Vector2), debugTextUVs.data());

	glBindVertexArray(textVAO);
	glDrawArrays(GL_TRIANGLES, 0, frameVertCount);
	glBindVertexArray(0);
}

Texture* GameTechRenderer::LoadTexture(const std::string& name) {
	OGLTexture*  tex = OGLTexture::TextureFromFile(name).release();
	if (FindTexHandleIndex(tex->GetObjectID()) == -1) {
		const GLuint64 handle = glGetTextureHandleARB(tex->GetObjectID());
		glMakeTextureHandleResidentARB(handle);
		mTextureHandles.push_back(std::pair<GLuint, GLuint64>(tex->GetObjectID(), handle));
		mLoadedTextures[name] = tex->GetObjectID();
	}
	
	return tex;
}

GLuint GameTechRenderer::LoadTextureGetID(const std::string& name) {
	if (mLoadedTextures.find(name) != mLoadedTextures.end()) {
		return mLoadedTextures[name];
	}
	Texture* texture = LoadTexture(name);
	return ((OGLTexture*)texture)->GetObjectID();
}

Texture* GameTechRenderer::LoadDebugTexture(const std::string& name) {
	OGLTexture* tex = OGLTexture::TextureFromFile(name).release();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex->GetObjectID());	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	if (FindTexHandleIndex(tex->GetObjectID()) == -1) {
		const GLuint64 handle = glGetTextureHandleARB(tex->GetObjectID());
		glMakeTextureHandleResidentARB(handle);
		mTextureHandles.push_back(std::pair<GLuint,GLuint64>(tex->GetObjectID(), handle));
	}
	return tex;
}

Shader* GameTechRenderer::LoadShader(const std::string& vertex, const std::string& fragment) {
	return new OGLShader(vertex, fragment);
}

MeshAnimation* NCL::CSC8503::GameTechRenderer::LoadAnimation(const std::string& name) {
	return new MeshAnimation(name);
}

MeshMaterial* NCL::CSC8503::GameTechRenderer::LoadMaterial(const std::string& name)
{
	return new MeshMaterial(name);
}

std::vector<int> NCL::CSC8503::GameTechRenderer::LoadMeshMaterial(Mesh& mesh, MeshMaterial& meshMaterial)
{
	std::vector<int> matTextures = std::vector<int>();
	for (int i = 0; i < mesh.GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = meshMaterial.GetMaterialForLayer(i);
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		GLuint texID = 0;

		if (filename) {
			string path = *filename;
			std::cout << path << std::endl;
			texID = LoadTextureGetID(path.c_str());
			std::cout << texID << endl;
		}
		matTextures.emplace_back(texID);

		filename = nullptr;
		matEntry->GetEntry("Normal", &filename);
		texID = 0;

		if (filename) {
			string path = *filename;
			std::cout << path << std::endl;
			texID = LoadTextureGetID(path.c_str());
			std::cout << texID << endl;
		}
		matTextures.emplace_back(texID);
	}
	return matTextures;
}

void GameTechRenderer::SetDebugStringBufferSizes(size_t newVertCount) {
	if (newVertCount > textCount) {
		textCount = newVertCount;

		glBindBuffer(GL_ARRAY_BUFFER, textVertVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector3), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, textColourVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector4), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, textTexVBO);
		glBufferData(GL_ARRAY_BUFFER, textCount * sizeof(Vector2), nullptr, GL_DYNAMIC_DRAW);

		debugTextPos.reserve(textCount);
		debugTextColours.reserve(textCount);
		debugTextUVs.reserve(textCount);

		glBindVertexArray(textVAO);

		glVertexAttribFormat(0, 3, GL_FLOAT, false, 0);
		glVertexAttribBinding(0, 0);
		glBindVertexBuffer(0, textVertVBO, 0, sizeof(Vector3));

		glVertexAttribFormat(1, 4, GL_FLOAT, false, 0);
		glVertexAttribBinding(1, 1);
		glBindVertexBuffer(1, textColourVBO, 0, sizeof(Vector4));

		glVertexAttribFormat(2, 2, GL_FLOAT, false, 0);
		glVertexAttribBinding(2, 2);
		glBindVertexBuffer(2, textTexVBO, 0, sizeof(Vector2));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}
}

void GameTechRenderer::SetDebugLineBufferSizes(size_t newVertCount) {
	if (newVertCount > lineCount) {
		lineCount = newVertCount;

		glBindBuffer(GL_ARRAY_BUFFER, lineVertVBO);
		glBufferData(GL_ARRAY_BUFFER, lineCount * sizeof(Debug::DebugLineEntry), nullptr, GL_DYNAMIC_DRAW);

		debugLineData.reserve(lineCount);

		glBindVertexArray(lineVAO);

		int realStride = sizeof(Debug::DebugLineEntry) / 2;

		glVertexAttribFormat(0, 3, GL_FLOAT, false, offsetof(Debug::DebugLineEntry, start));
		glVertexAttribBinding(0, 0);
		glBindVertexBuffer(0, lineVertVBO, 0, realStride);

		glVertexAttribFormat(1, 4, GL_FLOAT, false, offsetof(Debug::DebugLineEntry, colourA));
		glVertexAttribBinding(1, 0);
		glBindVertexBuffer(1, lineVertVBO, sizeof(Vector4), realStride);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}
}

void GameTechRenderer::SetUIiconBufferSizes(size_t newVertCount) {
	if (newVertCount > 0) {

		glBindBuffer(GL_ARRAY_BUFFER, iconVertVBO);
		glBufferData(GL_ARRAY_BUFFER, newVertCount * sizeof(Vector3), nullptr, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, iconTexVBO);
		glBufferData(GL_ARRAY_BUFFER, newVertCount * sizeof(Vector2), nullptr, GL_DYNAMIC_DRAW);

		UIiconPos.reserve(newVertCount);
		UIiconUVs.reserve(newVertCount);

		glBindVertexArray(iconVAO);

		glVertexAttribFormat(0, 3, GL_FLOAT, false, 0);
		glVertexAttribBinding(0, 0);
		glBindVertexBuffer(0, iconVertVBO, 0, sizeof(Vector3));

		glVertexAttribFormat(1, 2, GL_FLOAT, false, 0);
		glVertexAttribBinding(1, 1);
		glBindVertexBuffer(1, iconTexVBO, 0, sizeof(Vector2));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}
}

void GameTechRenderer::AddLight(Light* lightPtr) {
	if (mLights.size() >= MAX_POSSIBLE_LIGHTS) return;
	mLights.push_back(lightPtr);

}

void GameTechRenderer::ClearLights() {
	for (int i = 0; i < mLights.size(); i++) {
		delete(mLights[i]);
	}
	mLights.clear();
}

int GameTechRenderer::FindTexHandleIndex(GLuint texId) {
	for (int i = 0; i < mTextureHandles.size(); i++) {
		if (texId == mTextureHandles[i].first) {
			return i;
		}
	}
	return -1;
}

int GameTechRenderer::FindTexHandleIndex(const OGLTexture* tex) {
	if (!tex) return -1;
	for (int i = 0; i < mTextureHandles.size(); i++) {
		if (tex->GetObjectID() == mTextureHandles[i].first) {
			return i;
		}
	}
	return -1;
}
#endif