#include "GameTechRenderer.h"
#include "GameObject.h"
#include "RenderObject.h"
#include "Camera.h"
#include "TextureLoader.h"
#include "MshLoader.h"
#include "UISystem.h"
#include "Mesh.h"

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

#define SHADOWSIZE 4096

Matrix4 biasMatrix = Matrix4::Translation(Vector3(0.5f, 0.5f, 0.5f)) * Matrix4::Scale(Vector3(0.5f, 0.5f, 0.5f));

GameTechRenderer::GameTechRenderer(GameWorld& world) : OGLRenderer(*Window::GetWindow()), gameWorld(world) {
	glEnable(GL_DEPTH_TEST);

	debugShader = new OGLShader("debug.vert", "debug.frag");
	shadowShader = new OGLShader("shadow.vert", "shadow.frag");
	mOutlineShader = new OGLShader("outline.vert", "outline.frag");
	iconShader = new OGLShader("UI.vert", "UI.frag");

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

	//Set up the light properties
	Vector4 lightColour = Vector4(0.8f, 0.8f, 0.5f, 1.0f);
	float lightRadius = 35.0f;
	Vector3 dir = Vector3(0, -1, 0);
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			Vector3 lightPosition = Vector3(-30 + 30.0f * i, -5.0f, -30 + 60.0f * j);
			if (j % 2 == 0)
			{
				PointLight* pointL = new PointLight(lightPosition, lightColour, lightRadius);
				//AddLight(pointL);
			}
			else
			{
				SpotLight* spotL = new SpotLight(dir, lightPosition, lightColour, lightRadius, 40, 2);
				//AddLight(spotL);
			}
		}
	}

	DirectionLight* dLight = new DirectionLight({ 0.2,-0.7,0.3 }, { 0.1,0.1,0.1,1 }, 200, { 0,50,0 });
	//AddLight(dLight);

	//Skybox!
	skyboxShader = new OGLShader("skybox.vert", "skybox.frag");
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

	Debug::CreateDebugFont("PressStart2P.fnt", *LoadTexture("PressStart2P.png"));

	SetDebugStringBufferSizes(10000);
	SetDebugLineBufferSizes(1000);
}

GameTechRenderer::~GameTechRenderer() {
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
	ClearLights();
}

void GameTechRenderer::LoadSkybox() {
	std::string filenames[6] = {
		"/Cubemap/skyrender0004.png",
		"/Cubemap/skyrender0001.png",
		"/Cubemap/skyrender0003.png",
		"/Cubemap/skyrender0006.png",
		"/Cubemap/skyrender0002.png",
		"/Cubemap/skyrender0005.png"
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

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GameTechRenderer::LoadDefRendShaders() {
	mPointLightShader = LoadShader("light.vert", "pointlight.frag");
	mSpotLightShader = LoadShader("light.vert", "spotlight.frag");
	mDirLightShader = LoadShader("light.vert", "dirlight.frag");
	mCombineShader = LoadShader("combine.vert", "combine.frag");
}

void GameTechRenderer::RenderFrame() {
	Matrix4 viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());
	mFrameFrustum = mFrameFrustum.FromViewProjMatrix(projMatrix * viewMatrix);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glClearColor(1, 1, 1, 1);
	BuildObjectList();
	SortObjectList();

	RenderCamera();
	RenderSkybox();
	DrawOutlinedObjects();
	glDisable(GL_CULL_FACE); //Todo - text indices are going the wrong way...
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	NewRenderLines();
	NewRenderText();
	const std::vector<UISystem::Icon*>& icons = mUi->GetIcons();
	for ( auto& i : icons) {
		RenderIcons(*i);
	}
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void GameTechRenderer::BuildObjectList() {
	mActiveObjects.clear();
	mOutlinedObjects.clear();

	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			if (o->IsRendered()) {
				RenderObject* rendObj = o->GetRenderObject();
				bool isInFrustum = mFrameFrustum.SphereInsideFrustum(o->GetTransform().GetPosition(), o->GetRenderObject()->GetCullSphereRadius());
				if (rendObj && isInFrustum && !rendObj->IsInstanced()) {
					rendObj->SetSqDistToCam(gameWorld.GetMainCamera().GetPosition());
					mActiveObjects.emplace_back(rendObj);
					if (rendObj->GetOutlined()) {
						mOutlinedObjects.emplace_back(rendObj);
					}
				}
			}
		}
	);
}

void GameTechRenderer::SortObjectList() {
	std::sort(mActiveObjects.begin(), mActiveObjects.end(), RenderObject::CompareBySqCamDist);
}

void GameTechRenderer::RenderShadowMap() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);

	glCullFace(GL_FRONT);

	BindShader(*shadowShader);
	int mvpLocation = glGetUniformLocation(shadowShader->GetProgramID(), "mvpMatrix");
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

	Matrix4 viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());

	BindShader(*skyboxShader);

	glUniformMatrix4fv(glGetUniformLocation(skyboxShader->GetProgramID(), "projMatrix"), 1, false, (float*)&projMatrix);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader->GetProgramID(), "viewMatrix"), 1, false, (float*)&viewMatrix);

	glUniform1i(glGetUniformLocation(skyboxShader->GetProgramID(), "cubeTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

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
	Matrix4 viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());
	FillGBuffer(viewMatrix, projMatrix);
	DrawLightVolumes(viewMatrix, projMatrix);
	CombineBuffers();
}

void GameTechRenderer::DrawWallsFloorsInstanced(Matrix4& viewMatrix, Matrix4& projMatrix) {
	for (int i = 0; i < MAX_INSTANCE_MESHES; i++) {
		if (!mInstanceTiles[i]) continue;
		RenderObject* rendObj = mInstanceTiles[i]->GetRenderObject();
		OGLShader* shader = (OGLShader*)rendObj->GetShader();
		BindShader(*shader);
		if (rendObj->GetAlbedoTexture()) {
			BindTextureToShader(*(OGLTexture*)rendObj->GetAlbedoTexture(), "mainTex", 0);

		}

		if (rendObj->GetNormalTexture()) {
			BindTextureToShader(*(OGLTexture*)rendObj->GetNormalTexture(), "normTex", 2);
		}

		int projLocation = glGetUniformLocation(shader->GetProgramID(), "projMatrix");
		int viewLocation = glGetUniformLocation(shader->GetProgramID(), "viewMatrix");
		int shadowLocation = glGetUniformLocation(shader->GetProgramID(), "shadowMatrix");
		int colourLocation = glGetUniformLocation(shader->GetProgramID(), "objectColour");
		int hasVColLocation = glGetUniformLocation(shader->GetProgramID(), "hasVertexColours");
		int hasTexLocation = glGetUniformLocation(shader->GetProgramID(), "hasTexture");
		int cameraLocation = glGetUniformLocation(shader->GetProgramID(), "cameraPos");
		int hasInstanceMatLocation = glGetUniformLocation(shader->GetProgramID(), "hasInstanceMatrix");
		int shadowTexLocation = glGetUniformLocation(shader->GetProgramID(), "shadowTex");

		Vector3 camPos = gameWorld.GetMainCamera().GetPosition();
		glUniform3fv(cameraLocation, 1, &camPos.x);

		glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
		glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);
		glUniform1i(shadowTexLocation, 1);

		Vector4 colour = rendObj->GetColour();
		glUniform4fv(colourLocation, 1, &colour.x);
		glUniform1i(hasVColLocation, !rendObj->GetMesh()->GetColourData().empty());
		glUniform1i(hasTexLocation, (OGLTexture*)rendObj->GetAlbedoTexture() ? 1 : 0);
		glUniform1i(hasInstanceMatLocation, 1);
		OGLMesh* mesh = (OGLMesh*)rendObj->GetMesh();
		BindMesh(*mesh);
		size_t layerCount = mesh->GetSubMeshCount();
		for (size_t b = 0; b < layerCount; ++b) {

			DrawBoundMesh((uint32_t)b, mesh->GetInstanceMatricesSize());
		}
	}
}

void GameTechRenderer::FillGBuffer(Matrix4& viewMatrix, Matrix4& projMatrix) {
	glBindFramebuffer(GL_FRAMEBUFFER, mGBufferFBO);
	glEnable(GL_STENCIL_TEST);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	if(!mInstanceTiles.empty()) DrawWallsFloorsInstanced(viewMatrix, projMatrix);

	OGLShader* activeShader = nullptr;
	int projLocation = 0;
	int viewLocation = 0;
	int modelLocation = 0;
	int colourLocation = 0;
	int hasVColLocation = 0;
	int hasTexLocation = 0;
	int shadowLocation = 0;
	int hasInstanceMatLocation = 0;
	int cameraLocation = 0;

	//TODO - PUT IN FUNCTION
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	for (const auto& i : mActiveObjects) {
		OGLShader* shader = (OGLShader*)(*i).GetShader();
		BindShader(*shader);
		if ((*i).GetAlbedoTexture()) {
			BindTextureToShader(*(OGLTexture*)(*i).GetAlbedoTexture(), "mainTex", 0);

		}

		if ((*i).GetNormalTexture()) {
			BindTextureToShader(*(OGLTexture*)(*i).GetNormalTexture(), "normTex", 2);
		}
		if ((*i).GetAnimation()) {
			glUniform1i(glGetUniformLocation(shader->GetProgramID(), "mainTex"), 3);
		}
		if (activeShader != shader) {
			projLocation = glGetUniformLocation(shader->GetProgramID(), "projMatrix");
			viewLocation = glGetUniformLocation(shader->GetProgramID(), "viewMatrix");
			modelLocation = glGetUniformLocation(shader->GetProgramID(), "modelMatrix");
			shadowLocation = glGetUniformLocation(shader->GetProgramID(), "shadowMatrix");
			colourLocation = glGetUniformLocation(shader->GetProgramID(), "objectColour");
			hasVColLocation = glGetUniformLocation(shader->GetProgramID(), "hasVertexColours");
			hasTexLocation = glGetUniformLocation(shader->GetProgramID(), "hasTexture");
			cameraLocation = glGetUniformLocation(shader->GetProgramID(), "cameraPos");
			hasInstanceMatLocation = glGetUniformLocation(shader->GetProgramID(), "hasInstanceMatrix");

			Vector3 camPos = gameWorld.GetMainCamera().GetPosition();
			glUniform3fv(cameraLocation, 1, &camPos.x);

			glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
			glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);
			int shadowTexLocation = glGetUniformLocation(shader->GetProgramID(), "shadowTex");
			glUniform1i(shadowTexLocation, 1);

			activeShader = shader;
		}

		Matrix4 modelMatrix = (*i).GetTransform()->GetMatrix();
		glUniformMatrix4fv(modelLocation, 1, false, (float*)&modelMatrix);

		Matrix4 fullShadowMat = shadowMatrix * modelMatrix;
		glUniformMatrix4fv(shadowLocation, 1, false, (float*)&fullShadowMat);

		Vector4 colour = i->GetColour();
		glUniform4fv(colourLocation, 1, &colour.x);

		glUniform1i(hasVColLocation, !(*i).GetMesh()->GetColourData().empty());

		glUniform1i(hasTexLocation, (OGLTexture*)(*i).GetAlbedoTexture() ? 1 : 0);
		glUniform1i(hasInstanceMatLocation, 0);


		//Animation basic draw
		if ((*i).GetAnimation()) {
			BindMesh((OGLMesh&)*(*i).GetMesh());
			mMesh = (*i).GetMesh();
			size_t layerCount = mMesh->GetSubMeshCount();
			for (size_t b = 0; b < layerCount; ++b) {
				glActiveTexture(GL_TEXTURE3);
				GLuint textureID = (*i).GetMatTextures()[b];
				glBindTexture(GL_TEXTURE_2D, textureID);
				glUniformMatrix4fv(glGetUniformLocation(shader->GetProgramID(), "joints"), (*i).GetFrameMatricesVec()[b].size(), false, (float*)(*i).GetFrameMatricesVec()[b].data());
				DrawBoundMesh((uint32_t)b);
			}
		}
		else
		{
			BindMesh((OGLMesh&)*(*i).GetMesh());
			size_t layerCount = (*i).GetMesh()->GetSubMeshCount();
			for (size_t b = 0; b < layerCount; ++b) {

				DrawBoundMesh((uint32_t)b);
			}

		}
	}
	glDisable(GL_STENCIL_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GameTechRenderer::DrawLightVolumes(Matrix4& viewMatrix, Matrix4& projMatrix) {
	glBindFramebuffer(GL_FRAMEBUFFER, mLightFBO);
	BindCommonLightDataToShader((OGLShader*)mPointLightShader, viewMatrix, projMatrix);
	BindCommonLightDataToShader((OGLShader*)mSpotLightShader, viewMatrix, projMatrix);
	BindCommonLightDataToShader((OGLShader*)mDirLightShader, viewMatrix, projMatrix);
	glBlendFunc(GL_ONE, GL_ONE);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);

	for (int i = 0; i < mLights.size(); i++) {
		BindSpecificLightDataToShader(mLights[i]);
		BindMesh(*mSphereMesh);
		DrawBoundMesh();
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glClearColor(0.2f, 0.2f, 0.2f, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GameTechRenderer::BindCommonLightDataToShader(OGLShader* shader, Matrix4& viewMatrix, Matrix4& projMatrix) {
	BindShader(*shader);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glUniform1i(glGetUniformLocation(shader->GetProgramID(), "normTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mGBufferNormalTex);

	glUniform1i(glGetUniformLocation(shader->GetProgramID(), "depthTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mGBufferDepthTex);

	Vector3 camPos = gameWorld.GetMainCamera().GetPosition();
	glUniform3fv(glGetUniformLocation(shader->GetProgramID(), "cameraPos"), 1, &camPos.x);
	glUniform2f(glGetUniformLocation(shader->GetProgramID(), "pixelSize"), 1.0f / hostWindow.GetScreenSize().x, 1.0f / hostWindow.GetScreenSize().y);

	Matrix4 invProjView = (projMatrix * viewMatrix).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgramID(), "inverseProjView"), 1, false, (float*)&invProjView);
	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgramID(), "projMatrix"), 1, false, (float*)&projMatrix);
	glUniformMatrix4fv(glGetUniformLocation(shader->GetProgramID(), "viewMatrix"), 1, false, (float*)&viewMatrix);
}

void GameTechRenderer::CombineBuffers() {
	OGLShader* shader = (OGLShader*)mCombineShader;
	BindShader(*shader);
	glUniform1i(glGetUniformLocation(shader->GetProgramID(), "albedoTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mGBufferColourTex);

	glUniform1i(glGetUniformLocation(shader->GetProgramID(), "albedoLight"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mLightAlbedoTex);

	glUniform1i(glGetUniformLocation(shader->GetProgramID(), "specularLight"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, mLightSpecularTex);
	BindMesh(*mQuad);
	DrawBoundMesh();
}

void GameTechRenderer::DrawOutlinedObjects() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glClear(GL_DEPTH_BUFFER_BIT);
	BindShader(*mOutlineShader);
	Matrix4 viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());
	glUniform1i(glGetUniformLocation(mOutlineShader->GetProgramID(), "depthTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mGBufferDepthTex);
	glUniformMatrix4fv(glGetUniformLocation(mOutlineShader->GetProgramID(), "projMatrix"), 1, false, (float*)&projMatrix);
	glUniformMatrix4fv(glGetUniformLocation(mOutlineShader->GetProgramID(), "viewMatrix"), 1, false, (float*)&viewMatrix);
	glUniform2f(glGetUniformLocation(mOutlineShader->GetProgramID(), "pixelSize"), 1.0f / hostWindow.GetScreenSize().x, 1.0f / hostWindow.GetScreenSize().y);

	for (int i = 0; i < mOutlinedObjects.size(); i++) {
		int location = glGetUniformLocation(mOutlineShader->GetProgramID(), "hasAnim");
		glUniform1i(location, mOutlinedObjects[i]->GetAnimation() ? 1 : 0);
		Matrix4 modelMatrix = mOutlinedObjects[i]->GetTransform()->GetMatrix();
		glUniformMatrix4fv(glGetUniformLocation(mOutlineShader->GetProgramID(), "modelMatrix"), 1, false, (float*)&modelMatrix);
		OGLMesh* mesh = (OGLMesh*)mOutlinedObjects[i]->GetMesh();
		BindMesh(*mesh);
		size_t layerCount = mesh->GetSubMeshCount();
		for (size_t b = 0; b < layerCount; ++b) {
			glActiveTexture(GL_TEXTURE3);
			if (mOutlinedObjects[i]->GetAnimation()) {
				GLuint textureID = mOutlinedObjects[i]->GetMatTextures()[b];
				glBindTexture(GL_TEXTURE_2D, textureID);
				glUniformMatrix4fv(glGetUniformLocation(mOutlineShader->GetProgramID(), "joints"), mOutlinedObjects[i]->GetFrameMatricesVec()[b].size(), false, (float*)mOutlinedObjects[i]->GetFrameMatricesVec()[b].data());
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
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint internalFormat = depth ? GL_DEPTH24_STENCIL8 : GL_RGBA8;
	GLuint format = depth ? GL_DEPTH_STENCIL : GL_RGBA;
	GLuint type = depth ? GL_UNSIGNED_INT_24_8 : GL_UNSIGNED_BYTE;

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, hostWindow.GetScreenSize().x, hostWindow.GetScreenSize().y, 0, format, type, NULL);
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

void GameTechRenderer::NewRenderLines() {
	const std::vector<Debug::DebugLineEntry>& lines = Debug::GetDebugLines();
	if (lines.empty()) {
		return;
	}

	Matrix4 viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());

	Matrix4 viewProj = projMatrix * viewMatrix;

	BindShader(*debugShader);
	int matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	GLuint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");
	glUniform1i(texSlot, 0);

	glUniformMatrix4fv(matSlot, 1, false, (float*)viewProj.array);

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

	BindShader(*iconShader);

	int iconVertCount = 6;

	UIiconPos.clear();
	UIiconUVs.clear();

	OGLTexture* t = (OGLTexture*)i.texture;
	BindTextureToShader(*t, "iconTex", t->GetObjectID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	mUi->BuildVerticesForIcon(i.position, i.length, i.height, UIiconPos, UIiconUVs);

	bool texSlot = glGetUniformLocation(iconShader->GetProgramID(), "isOn");
	glUniform1i(texSlot, i.isAppear);

	Matrix4 proj = Matrix4::Orthographic(0.0, 100.0f, 100, 0, -1.0f, 1.0f);
	//0.02, 0, 0, 0
	//0, 0.02, 0, 0
	//0, 0, -1, 0;
	//-1, 1, 0, 1

	int matSlot = glGetUniformLocation(iconShader->GetProgramID(), "viewProjMatrix");
	glUniformMatrix4fv(matSlot, 1, false, (float*)proj.array);

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

	BindShader(*debugShader);

	OGLTexture* t = (OGLTexture*)Debug::GetDebugFont()->GetTexture();

	if (t) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, t->GetObjectID());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		BindTextureToShader(*t, "mainTex", 0);
	}
	Matrix4 proj = Matrix4::Orthographic(0.0, 100.0f, 100, 0, -1.0f, 1.0f);

	int matSlot = glGetUniformLocation(debugShader->GetProgramID(), "viewProjMatrix");
	glUniformMatrix4fv(matSlot, 1, false, (float*)proj.array);

	GLuint texSlot = glGetUniformLocation(debugShader->GetProgramID(), "useTexture");
	glUniform1i(texSlot, 1);

	debugTextPos.clear();
	debugTextColours.clear();
	debugTextUVs.clear();

	int frameVertCount = 0;
	for (const auto& s : strings) {
		frameVertCount += Debug::GetDebugFont()->GetVertexCountForString(s.data);
	}
	SetDebugStringBufferSizes(frameVertCount);

	for (const auto& s : strings) {
		float size = 20.0f;
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
	return OGLTexture::TextureFromFile(name).release();
}

Shader* GameTechRenderer::LoadShader(const std::string& vertex, const std::string& fragment) {
	return new OGLShader(vertex, fragment);
}

MeshAnimation* NCL::CSC8503::GameTechRenderer::LoadAnimation(const std::string& name)
{
	return new MeshAnimation(name);
}

MeshMaterial* NCL::CSC8503::GameTechRenderer::LoadMaterial(const std::string& name)
{
	return new MeshMaterial(name);
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

	mLights.push_back(lightPtr);
}

void GameTechRenderer::ClearLights() {
	for (int i = 0; i < mLights.size(); i++) {
		delete(mLights[i]);
	}
	mLights.clear();
}

void GameTechRenderer::BindSpecificLightDataToShader(Light* l)
{
	Light::Type type = l->GetType();
	if (type == Light::Point) {
		SendPointLightDataToShader((OGLShader*)mPointLightShader, (PointLight*)l);
	}
	else if (type == Light::Spot) {
		SendSpotLightDataToShader((OGLShader*)mSpotLightShader, (SpotLight*)l);
	}
	else if (type == Light::Direction) {
		SendDirLightDataToShader((OGLShader*)mDirLightShader, (DirectionLight*)l);
	}
}

void GameTechRenderer::SendPointLightDataToShader(OGLShader* shader, PointLight* l) {
	BindShader(*shader);
	int lightPosLocation = 0;
	int lightColourLocation = 0;
	int lightRadiusLocation = 0;

	lightPosLocation = glGetUniformLocation(shader->GetProgramID(), "lightPos");
	lightColourLocation = glGetUniformLocation(shader->GetProgramID(), "lightColour");
	lightRadiusLocation = glGetUniformLocation(shader->GetProgramID(), "lightRadius");
	glUniform3fv(lightPosLocation, 1, l->GetPositionAddress());
	glUniform4fv(lightColourLocation, 1, l->GetColourAddress());
	glUniform1f(lightRadiusLocation, l->GetRadius());
}

void GameTechRenderer::SendDirLightDataToShader(OGLShader* shader, DirectionLight* l) {
	BindShader(*shader);
	int lightColourLocation = 0;
	int dirLightDirectionLocation = 0;
	int lightRadiusLocation = 0;
	int dirCentreLocation = 0;

	dirCentreLocation = glGetUniformLocation(shader->GetProgramID(), "lightPos");
	lightColourLocation = glGetUniformLocation(shader->GetProgramID(), "lightColour");
	dirLightDirectionLocation = glGetUniformLocation(shader->GetProgramID(), "lightDirection");
	lightRadiusLocation = glGetUniformLocation(shader->GetProgramID(), "lightRadius");
	glUniform3fv(dirLightDirectionLocation, 1, l->GetDirectionAddress());
	glUniform4fv(lightColourLocation, 1, l->GetColourAddress());
	glUniform1f(lightRadiusLocation, l->GetRadius());
	glUniform3fv(dirCentreLocation, 1, l->GetCentreAddress());
}

void GameTechRenderer::SendSpotLightDataToShader(OGLShader* shader, SpotLight* l) {
	BindShader(*shader);
	int lightPosLocation = 0;
	int lightColourLocation = 0;
	int lightRadiusLocation = 0;
	int spotLightDirLocation = 0;
	int minDotProdLocation = 0;
	int dimDotProdLocation = 0;

	lightPosLocation = glGetUniformLocation(shader->GetProgramID(), "lightPos");
	lightColourLocation = glGetUniformLocation(shader->GetProgramID(), "lightColour");
	lightRadiusLocation = glGetUniformLocation(shader->GetProgramID(), "lightRadius");
	spotLightDirLocation = glGetUniformLocation(shader->GetProgramID(), "spotlightDir");
	minDotProdLocation = glGetUniformLocation(shader->GetProgramID(), "minDotProd");
	dimDotProdLocation = glGetUniformLocation(shader->GetProgramID(), "dimDotProd");
	glUniform3fv(lightPosLocation, 1, l->GetPositionAddress());
	glUniform4fv(lightColourLocation, 1, l->GetColourAddress());
	glUniform1f(lightRadiusLocation, l->GetRadius());
	glUniform3fv(spotLightDirLocation, 1, l->GetDirectionAddress());
	glUniform1f(minDotProdLocation, l->GetDotProdMin());
	glUniform1f(dimDotProdLocation, l->GetDimProdMin());
}


