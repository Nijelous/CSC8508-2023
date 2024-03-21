#include "GameTechAGCRenderer.h"

#include <algorithm>

#include "GameObject.h"
#include "RenderObject.h"
#include "Camera.h"
#include "TextureLoader.h"
#include "MshLoader.h"
#include "../PS5Core/AGCMesh.h"
#include "../PS5Core/AGCTexture.h"
#include "../PS5Core/AGCShader.h"

#include "../CSC8503CoreClasses/Debug.h"

class SpotLight;
class PointLight;
class DirectionLight;

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;
using namespace PS5;

const int SHADOW_SIZE = 8192;
const int FRAMES_IN_FLIGHT = 2;

const int BINDLESS_TEX_COUNT = 128;
const int SKINNED_MESH_COUNT = 128;
const int LIGHT_BUFFER_COUNT = 1;
constexpr int BINDLESS_BUFFER_COUNT = SKINNED_MESH_COUNT + LIGHT_BUFFER_COUNT;


const size_t LINE_STRIDE = sizeof(Vector4) + sizeof(Vector4);
const size_t TEXT_STRIDE = sizeof(Vector2) + sizeof(Vector2) + sizeof(Vector4);

GameTechAGCRenderer::GameTechAGCRenderer(GameWorld& world)
	: AGCRenderer(*Window::GetWindow()), gameWorld(world) {
	SceError error = SCE_OK;
	bindlessTextures = (sce::Agc::Core::Texture*)allocator.Allocate(BINDLESS_TEX_COUNT * sizeof(sce::Agc::Core::Texture), sce::Agc::Alignment::kBuffer);
	sce::Agc::Core::BufferSpec texSpec;
	texSpec.initAsRegularBuffer(bindlessTextures, sizeof(sce::Agc::Core::Texture), BINDLESS_TEX_COUNT);
	error = sce::Agc::Core::initialize(&textureBuffer, &texSpec);

	bindlessBuffers = (sce::Agc::Core::Buffer*)allocator.Allocate(BINDLESS_BUFFER_COUNT * sizeof(sce::Agc::Core::Buffer), sce::Agc::Alignment::kBuffer);
	sce::Agc::Core::BufferSpec buffSpec;
	buffSpec.initAsRegularBuffer(bindlessBuffers, sizeof(sce::Agc::Core::Buffer), BINDLESS_BUFFER_COUNT);
	error = sce::Agc::Core::initialize(&arrayBuffer, &buffSpec);
	bufferCount = 1; //We skip over index 0, makes some selection logic easier later

	skyboxTexture = (AGCTexture*)LoadTexture("Skybox.dds");
	defaultTexture = (AGCTexture*)LoadTexture("doge.png");

	quadMesh = new AGCMesh();
	CreateQuad(quadMesh);
	quadMesh->UploadToGPU(this);
	mSphereMesh = (AGCMesh*)LoadMesh("Sphere.msh");

	skinningCompute = new AGCShader("Skinning_c.ags", allocator);
	gammaCompute = new AGCShader("Gamma_c.ags", allocator);

	defaultVertexShader = new AGCShader("Tech_vv.ags", allocator);
	defaultPixelShader = new AGCShader("Tech_p.ags", allocator);

	shadowVertexShader = new AGCShader("Shadow_vv.ags", allocator);
	shadowPixelShader = new AGCShader("Shadow_p.ags", allocator);

	skyboxVertexShader = new AGCShader("Skybox_vv.ags", allocator);
	skyboxPixelShader = new AGCShader("Skybox_p.ags", allocator);

	debugLineVertexShader = new AGCShader("DebugLine_vv.ags", allocator);
	debugLinePixelShader = new AGCShader("DebugLine_p.ags", allocator);

	debugTextVertexShader = new AGCShader("DebugText_vv.ags", allocator);
	debugTextPixelShader = new AGCShader("DebugText_p.ags", allocator);

	lightVertexShader = new AGCShader("light_vv.ags", allocator);
	lightPixelShader = new AGCShader("light_p.ags", allocator);

	combineVertexShader = new AGCShader("Combine_vv.ags", allocator);
	combinePixelShader = new AGCShader("Combine_p.ags", allocator);

	allFrames = new FrameData[FRAMES_IN_FLIGHT];
	for (int i = 0; i < FRAMES_IN_FLIGHT; ++i) {

		{//We store scene object matrices etc in a big UBO
			allFrames[i].data.dataStart = (char*)allocator.Allocate(1024 * 1024 * 64, sce::Agc::Alignment::kBuffer);
			allFrames[i].data.data = allFrames[i].data.dataStart;

			sce::Agc::Core::BufferSpec bufSpec;
			bufSpec.initAsConstantBuffer(allFrames[i].data.dataStart, sizeof(ShaderConstants));

			SceError error = sce::Agc::Core::initialize(&allFrames[i].constantBuffer, &bufSpec);
		}
	}
	currentFrameIndex = 0;
	currentFrame = &allFrames[currentFrameIndex];

	Debug::CreateDebugFont("PressStart2P.fnt", *LoadTexture("PressStart2P.png"));

	mGBuffDepthTarget = CreateDepthBufferTarget(Window::GetWindow()->GetScreenSize().x, Window::GetWindow()->GetScreenSize().y);
	mGBuffDepthTex = CreateFrameBufferTextureSlot("depthTex");

	screenTarget = CreateColourBufferTarget(Window::GetWindow()->GetScreenSize().x, Window::GetWindow()->GetScreenSize().y, true);
	mGBuffAlbedoTarget = CreateColourBufferTarget(Window::GetWindow()->GetScreenSize().x, Window::GetWindow()->GetScreenSize().y, true);
	mGBuffNormalTarget = CreateColourBufferTarget(Window::GetWindow()->GetScreenSize().x, Window::GetWindow()->GetScreenSize().y, true)
		.setSlot(1);

	mDiffLightTarget = CreateColourBufferTarget(Window::GetWindow()->GetScreenSize().x, Window::GetWindow()->GetScreenSize().y, true);
	mSpecLightTarget = CreateColourBufferTarget(Window::GetWindow()->GetScreenSize().x, Window::GetWindow()->GetScreenSize().y, true)
		.setSlot(1);

	mCombineTarget = CreateColourBufferTarget(Window::GetWindow()->GetScreenSize().x, Window::GetWindow()->GetScreenSize().y, true);

	screenTex = CreateFrameBufferTextureSlot("Screen");
	mGBuffAlbedoTex = CreateFrameBufferTextureSlot("albedoTex");
	mGBuffNormalTex = CreateFrameBufferTextureSlot("normalTex");
	mDiffLightTex = CreateFrameBufferTextureSlot("diffLight");
	mSpecLightTex = CreateFrameBufferTextureSlot("specLightTex");
	mCombineTex = CreateFrameBufferTextureSlot("combineTex");

	error = sce::Agc::Core::translate(screenTex->GetAGCPointer(), &screenTarget, sce::Agc::Core::RenderTargetComponent::kData);
	error = sce::Agc::Core::translate(mGBuffAlbedoTex->GetAGCPointer(), &mGBuffAlbedoTarget, sce::Agc::Core::RenderTargetComponent::kData);
	error = sce::Agc::Core::translate(mGBuffNormalTex->GetAGCPointer(), &mGBuffNormalTarget, sce::Agc::Core::RenderTargetComponent::kData);
	error = sce::Agc::Core::translate(mDiffLightTex->GetAGCPointer(), &mDiffLightTarget, sce::Agc::Core::RenderTargetComponent::kData);
	error = sce::Agc::Core::translate(mSpecLightTex->GetAGCPointer(), &mSpecLightTarget, sce::Agc::Core::RenderTargetComponent::kData);
	error = sce::Agc::Core::translate(mCombineTex->GetAGCPointer(), &mCombineTarget, sce::Agc::Core::RenderTargetComponent::kData);
	shadowSampler.init()
		.setXyFilterMode(
			sce::Agc::Core::Sampler::FilterMode::kPoint,	//magnification
			sce::Agc::Core::Sampler::FilterMode::kPoint		//minificaction
		)
		.setMipFilterMode(sce::Agc::Core::Sampler::MipFilterMode::kPoint);
}

GameTechAGCRenderer::~GameTechAGCRenderer() {

}

Mesh* GameTechAGCRenderer::LoadMesh(const std::string& name) {
	AGCMesh* m = new AGCMesh();

	if (name.find(".gltf") != std::string::npos) {
		bool a = true;
	}
	else if (name.find(".msh") != std::string::npos) {
		MshLoader::LoadMesh(name, *m);
	}

	m->UploadToGPU(this);

	return m;
}

NCL::PS5::AGCTexture* GameTechAGCRenderer::CreateFrameBufferTextureSlot(const std::string& name) {
	uint32_t index = textureMap.size();
	AGCTexture* t = new AGCTexture(allocator);

	textureMap.insert({ name, t });
	t->SetAssetID(index);
	bindlessTextures[t->GetAssetID()] = *t->GetAGCPointer();

	return t;
}

Texture* GameTechAGCRenderer::LoadTexture(const std::string& name) {
	auto found = textureMap.find(name);
	if (found != textureMap.end()) {
		return found->second;
	}
	AGCTexture* t = new AGCTexture(name, allocator);

	t->SetAssetID(textureMap.size());
	textureMap.insert({ name, t });

	bindlessTextures[t->GetAssetID()] = *t->GetAGCPointer();

	mLoadedTextures[name] = t->GetAssetID();

	return t;
}

Shader* GameTechAGCRenderer::LoadShader(const std::string& vertex, const std::string& fragment) {
	return nullptr;
}

MeshAnimation* GameTechAGCRenderer::LoadAnimation(const std::string& name) {
	return new MeshAnimation(name);
}

MeshMaterial* GameTechAGCRenderer::LoadMaterial(const std::string& name) {
	return new MeshMaterial(name);
}

int GameTechAGCRenderer::LoadTextureGetID(const std::string& name) {
	if (mLoadedTextures.find(name) != mLoadedTextures.end()) {
		return mLoadedTextures[name];
	}
	Texture* texture = LoadTexture(name);
	return ((AGCTexture*)texture)->GetAssetID();
}

std::vector<int> GameTechAGCRenderer::LoadMeshMaterial(Mesh& mesh, MeshMaterial& meshMaterial) {
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

void GameTechAGCRenderer::RenderFrame() {
	currentFrame = &allFrames[currentFrameIndex];

	currentFrame->data.Reset();

	currentFrame->globalDataOffset = 0;
	currentFrame->objectStateOffset = sizeof(ShaderConstants);
	currentFrame->debugLinesOffset = currentFrame->objectStateOffset; //We'll be pushing that out later
	
	//Step 1: Write the frame's constant data to the buffer
	WriteRenderPassConstants();
	//Step 2: Walk the object list and build up the object set and required buffer memory
	UpdateObjectList();
	//Step 3: Run a compute shader for every skinned mesh to generate its positions, tangents, and normals
	GPUSkinningPass();
	//Step 4: Go through the geometry and draw it to a shadow map
	//NO THANK YOU

	//Step 5: Draw a skybox to our main scene render target
	
	//Step 6: Draw the scene to our main scene render target
	DeferredRenderingPass();
	SkyboxPass();

	//Step 7: Draw the debug data to the main scene render target
	UpdateDebugData();
	RenderDebugLines();
	RenderDebugText();
	//Step 8: Draw the main scene render target to the screen with a compute shader
	DisplayRenderPass(); //Puts our scene on screen, uses a compute

	currentFrameIndex = (currentFrameIndex + 1) % FRAMES_IN_FLIGHT;
}

/*
This method builds a struct that

*/
void GameTechAGCRenderer::WriteRenderPassConstants() {
	ShaderConstants frameData;

	frameData.cameraPos = gameWorld.GetMainCamera().GetPosition();

	frameData.viewMatrix = gameWorld.GetMainCamera().BuildViewMatrix();
	frameData.projMatrix = gameWorld.GetMainCamera().BuildProjectionMatrix(hostWindow.GetScreenAspect());

	frameData.viewProjMatrix = frameData.projMatrix * frameData.viewMatrix;

	frameData.inverseViewProjMatrix = frameData.viewProjMatrix.Inverse();
	frameData.inverseViewMatrix = frameData.viewMatrix.Inverse();
	frameData.inverseProjMatrix = frameData.projMatrix.Inverse();

	frameData.orthoMatrix = Matrix4::Orthographic(0.0f, 100.0f, 100.0f, 0.0f, -1.0f, 1.0f);
	frameData.pixelSize = Vector2(1.0f / hostWindow.GetScreenSize().x, 1.0f / hostWindow.GetScreenSize().y);

	frameData.gBuffAlbedoIndex = mGBuffAlbedoTex->GetAssetID();
	frameData.gBuffNormalIndex = mGBuffNormalTex->GetAssetID();
	frameData.gBuffDepthIndex = mGBuffDepthTex->GetAssetID();
	frameData.specularLightIndex = mSpecLightTex->GetAssetID();
	frameData.diffuseLightIndex = mDiffLightTex->GetAssetID();

	currentFrame->data.WriteData<ShaderConstants>(frameData); //Let's start filling up our frame data!

	currentFrame->data.AlignData(128);
	currentFrame->objectStateOffset = currentFrame->data.bytesWritten;
}

void GameTechAGCRenderer::DrawObjects() {
	if (activeObjects.empty()) {
		return;
	}
	uint32_t startingIndex = 0;

	AGCMesh* prevMesh = (AGCMesh*)activeObjects[0]->GetMesh();
	int previousIgnoredSubMesh = -1;
	int instanceCount = 0;

	bool skipInstance = false;

	for (int i = 0; i < activeObjects.size(); ++i) {
		AGCMesh* objectMesh = (AGCMesh*)activeObjects[i]->GetMesh();

		//The new mesh is different than previous meshes, flush out the old list
		if (prevMesh != objectMesh || skipInstance) {
			prevMesh->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));
			Temp* tempStruct = static_cast<Temp*>(frameContext->m_dcb.allocateTopDown(sizeof(Temp), sce::Agc::Alignment::kBuffer));
			tempStruct->objID = startingIndex;

			frameContext->m_dcb.setNumInstances(instanceCount);
			// draw by submesh
			for (size_t x = 0; x < prevMesh->GetSubMeshCount(); x++) {
				if (x != previousIgnoredSubMesh) {
					tempStruct->subMeshID = x;
					frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs).setUserSrtBuffer(tempStruct, 2);
					frameContext->drawIndex(prevMesh->GetSubMesh(x)->count, prevMesh->GetAGCIndexData() + (prevMesh->GetSubMesh(x)->start * sizeof(int)));
				}
			}
			frameContext->m_dcb.setNumInstances(1);
			prevMesh = objectMesh;
			previousIgnoredSubMesh = activeObjects[i]->GetIgnoredSubmeshID();
			instanceCount = 0;
			startingIndex = i;
		}
		if (i == activeObjects.size() - 1) {
			objectMesh->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));

			Temp* tempStruct = static_cast<Temp*>(frameContext->m_dcb.allocateTopDown(sizeof(Temp), sce::Agc::Alignment::kBuffer));
			tempStruct->objID = startingIndex;

			if (prevMesh == objectMesh) {
				instanceCount++;
			}

			frameContext->m_dcb.setNumInstances(instanceCount);
			// draw by submesh
			for (size_t x = 0; x < objectMesh->GetSubMeshCount(); x++) {
				if (x != activeObjects[i]->GetIgnoredSubmeshID()) {
					if (x != activeObjects[i]->GetIgnoredSubmeshID()) {
						tempStruct->subMeshID = x;
						frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs).setUserSrtBuffer(tempStruct, 2);
						frameContext->drawIndex(objectMesh->GetSubMesh(x)->count, objectMesh->GetAGCIndexData() + (objectMesh->GetSubMesh(x)->start * sizeof(int)));
					}
				}
			}
			frameContext->m_dcb.setNumInstances(1);
		}
		else {
			instanceCount++;
		}
	}
}

void GameTechAGCRenderer::GPUSkinningPass() {
	if (frameJobs.empty()) {
		return;
	}

	frameContext->setCsShader(skinningCompute->GetAGCPointer());

	sce::Agc::DispatchModifier modifier = skinningCompute->GetAGCPointer()->m_specials->m_dispatchModifier;


	for (int i = 0; i < frameJobs.size(); i++) {
		NCL::PS5::AGCMesh* m = (AGCMesh*)frameJobs[i].object->GetMesh();
		size_t layerCount = m->GetSubMeshCount();

		for (size_t b = 0; b < layerCount; ++b) {
			sce::Agc::Core::Buffer inputBuffers[6];

			if (!m->GetAGCBuffer(VertexAttribute::Positions, inputBuffers[0]) ||
				!m->GetAGCBuffer(VertexAttribute::Normals, inputBuffers[1]) ||
				!m->GetAGCBuffer(VertexAttribute::Tangents, inputBuffers[2]) ||
				!m->GetAGCBuffer(VertexAttribute::JointWeights, inputBuffers[3]) ||
				!m->GetAGCBuffer(VertexAttribute::JointIndices, inputBuffers[4])) {
				continue;
			}

			char* offset = currentFrame->data.data;

			const std::vector<Matrix4>& skeleton = frameJobs[i].object->GetFrameMatricesVec()[b];
			currentFrame->data.WriteData((void*)skeleton.data(), sizeof(Matrix4) * skeleton.size());

			sce::Agc::Core::BufferSpec bufSpec;
			bufSpec.initAsRegularBuffer(offset, sizeof(Matrix4), skeleton.size());
			SceError error = sce::Agc::Core::initialize(&inputBuffers[5], &bufSpec);

			inputBuffers[5].setFormat(sce::Agc::Core::Buffer::Format::k32_32_32_32Float);

			frameContext->m_bdr.getStage(sce::Agc::ShaderType::kCs)
				.setBuffers(0, 6, inputBuffers)
				.setRwBuffers(0, 1, &bindlessBuffers[frameJobs[i].outputIndex]);

			uint32_t threadCount = (m->GetVertexCount() + 63) / 64;
			frameContext->m_dcb.dispatch(threadCount, 1, 1, modifier);
		}
	}
	//TODO fence
	frameJobs.clear();

	frameContext->setCsShader(nullptr);
}


void GameTechAGCRenderer::SkyboxPass() {
	frameContext->setShaders(nullptr, skyboxVertexShader->GetAGCPointer(), skyboxPixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);

	sce::Agc::CxViewport viewPort;
	sce::Agc::Core::setViewport(&viewPort, SCREENWIDTH, SCREENHEIGHT, 0, 0, -1.0f, 1.0f);
	frameContext->m_sb.setState(viewPort);

	sce::Agc::CxRenderTargetMask rtMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0xFF);
	frameContext->m_sb.setState(rtMask);
	frameContext->m_sb.setState(mCombineTarget);

	frameContext->m_sb.setState(mGBuffDepthTarget);

	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init();
	depthControl.setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable);
	depthControl.setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kAlways);
	depthControl.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kDisable);
	frameContext->m_sb.setState(depthControl);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setSamplers(0, 1, &defaultSampler)
		.setTextures(1, 1, skyboxTexture->GetAGCPointer())
		.setBuffers(2, 1, &textureBuffer)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer);

	quadMesh->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));
	DrawBoundMesh(*frameContext, *quadMesh);
}

void GameTechAGCRenderer::ShadowmapPass() {
	sce::Agc::Toolkit::Result tk1 = sce::Agc::Toolkit::clearDepthRenderTargetCs(&frameContext->m_dcb, &shadowTarget);
	sce::Agc::Toolkit::Result wat = frameContext->resetToolkitChangesAndSyncToGl2(tk1);

	frameContext->setShaders(nullptr, shadowVertexShader->GetAGCPointer(), shadowPixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);

	sce::Agc::CxViewport viewPort;
	sce::Agc::Core::setViewport(&viewPort, SHADOW_SIZE, SHADOW_SIZE, 0, 0, -1.0f, 1.0f);
	frameContext->m_sb.setState(viewPort);

	sce::Agc::CxRenderTargetMask rtMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0x0);
	frameContext->m_sb.setState(rtMask);
	frameContext->m_sb.setState(shadowTarget);

	sce::Agc::CxDepthStencilControl depthControl;
	depthControl
		.init()
		.setDepth(sce::Agc::CxDepthStencilControl::Depth::kEnable)
		.setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kLessEqual)
		.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kEnable);

	frameContext->m_sb.setState(depthControl);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->objectBuffer)
		.setBuffers(1, 1, &arrayBuffer);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setSamplers(0, 1, &defaultSampler)
		.setBuffers(1, 1, &textureBuffer);

	DrawObjects();

	sce::Agc::Core::gpuSyncEvent(&frameContext->m_dcb, sce::Agc::Core::SyncWaitMode::kDrainGraphics, sce::Agc::Core::SyncCacheOp::kFlushCompressedDepthBufferForTexture);

	const bool htileTC = (shadowTarget.getTextureCompatiblePlaneCompression() == sce::Agc::CxDepthRenderTarget::TextureCompatiblePlaneCompression::kEnable);
	const sce::Agc::Core::MaintainCompression maintainCompression = htileTC ? sce::Agc::Core::MaintainCompression::kEnable : sce::Agc::Core::MaintainCompression::kDisable;

	sce::Agc::Core::translate(&bindlessTextures[shadowMap->GetAssetID()], &shadowTarget, sce::Agc::Core::DepthRenderTargetComponent::kDepth, maintainCompression);

}

void GameTechAGCRenderer::DeferredRenderingPass() {

	FillGBuffer();
	if (mLights.size() > 0) DrawLightVolumes();
	CombineBuffers();
}


void GameTechAGCRenderer::FillGBuffer() {
	sce::Agc::Toolkit::Result tk1 = sce::Agc::Toolkit::clearDepthRenderTargetCs(&frameContext->m_dcb, &mGBuffDepthTarget);
	sce::Agc::Toolkit::Result wat = frameContext->resetToolkitChangesAndSyncToGl2(tk1);

	frameContext->setShaders(nullptr, defaultVertexShader->GetAGCPointer(), defaultPixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);

	sce::Agc::CxViewport viewPort;
	sce::Agc::Core::setViewport(&viewPort, SCREENWIDTH, SCREENHEIGHT, 0, 0, -1.0f, 100.0f);
	frameContext->m_sb.setState(viewPort);

	sce::Agc::CxRenderTargetMask rtMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0xFF);
	rtMask.setMask(1, 0xFF);

	frameContext->m_sb.setState(rtMask);
	frameContext->m_sb.setState(mGBuffAlbedoTarget);
	frameContext->m_sb.setState(mGBuffNormalTarget);
	frameContext->m_sb.setState(mGBuffDepthTarget);

	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init();
	depthControl.setDepth(sce::Agc::CxDepthStencilControl::Depth::kEnable);
	depthControl.setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kLessEqual);
	depthControl.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kEnable);
	frameContext->m_sb.setState(depthControl);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->objectBuffer)
		.setBuffers(1, 1, &arrayBuffer);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &textureBuffer)
		.setSamplers(0, 1, &defaultSampler);
	DrawObjects();

	sce::Agc::Core::gpuSyncEvent(&frameContext->m_dcb, sce::Agc::Core::SyncWaitMode::kDrainGraphics, sce::Agc::Core::SyncCacheOp::kFlushUncompressedColorBufferForTexture);
	const bool htileTC = (mGBuffDepthTarget.getTextureCompatiblePlaneCompression() == sce::Agc::CxDepthRenderTarget::TextureCompatiblePlaneCompression::kEnable);
	const sce::Agc::Core::MaintainCompression maintainCompression = htileTC ? sce::Agc::Core::MaintainCompression::kEnable : sce::Agc::Core::MaintainCompression::kDisable;
	sce::Agc::Core::translate(&bindlessTextures[mGBuffDepthTex->GetAssetID()], &mGBuffDepthTarget, sce::Agc::Core::DepthRenderTargetComponent::kDepth, maintainCompression);

	sce::Agc::Core::gpuSyncEvent(&frameContext->m_dcb, sce::Agc::Core::SyncWaitMode::kDrainGraphics, sce::Agc::Core::SyncCacheOp::kFlushCompressedDepthBufferForTexture);
	sce::Agc::Core::translate(&bindlessTextures[mGBuffAlbedoTex->GetAssetID()], &mGBuffAlbedoTarget, sce::Agc::Core::RenderTargetComponent::kData);
	sce::Agc::Core::translate(&bindlessTextures[mGBuffNormalTex->GetAssetID()], &mGBuffNormalTarget, sce::Agc::Core::RenderTargetComponent::kData);

}

void GameTechAGCRenderer::DrawLightVolumes() {
	sce::Agc::Core::Encoder::EncoderValue clearColor = sce::Agc::Core::Encoder::encode(backBuffers[currentSwap].spec.getFormat(), { 0,0,0,1 });
	sce::Agc::Toolkit::Result tk1 = sce::Agc::Toolkit::clearRenderTargetCs(&frameContext->m_dcb, &mDiffLightTarget, sce::Agc::Toolkit::RenderTargetClearOp::kAuto);
	tk1 = sce::Agc::Toolkit::clearRenderTargetCs(&frameContext->m_dcb, &mSpecLightTarget, sce::Agc::Toolkit::RenderTargetClearOp::kAuto);
	sce::Agc::Toolkit::Result wat = frameContext->resetToolkitChangesAndSyncToGl2(tk1);

	frameContext->setShaders(nullptr, lightVertexShader->GetAGCPointer(), lightPixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);

	sce::Agc::CxViewport viewPort;
	sce::Agc::Core::setViewport(&viewPort, SCREENWIDTH, SCREENHEIGHT, 0, 0, -1.0f, 1.0f);
	frameContext->m_sb.setState(viewPort);

	sce::Agc::CxRenderTargetMask rtMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0xFF);
	rtMask.setMask(1, 0xFF);

	frameContext->m_sb.setState(rtMask);
	frameContext->m_sb.setState(mDiffLightTarget);
	frameContext->m_sb.setState(mSpecLightTarget);

	sce::Agc::CxPrimitiveSetup primState;
	primState.init();
	primState.setCullFace(sce::Agc::CxPrimitiveSetup::CullFace::kFront);
	frameContext->m_sb.setState(primState);

	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init();
	depthControl.setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kAlways);
	depthControl.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kDisable);
	frameContext->m_sb.setState(depthControl);

	sce::Agc::CxBlendControl blendControl;
	blendControl.init();
	blendControl.setBlend(sce::Agc::CxBlendControl::Blend::kEnable)
		.setAlphaBlendFunc(sce::Agc::CxBlendControl::AlphaBlendFunc::kAdd)
		.setColorBlendFunc(sce::Agc::CxBlendControl::ColorBlendFunc::kAdd)
		.setAlphaSourceMultiplier(sce::Agc::CxBlendControl::AlphaSourceMultiplier::kOne)
		.setColorSourceMultiplier(sce::Agc::CxBlendControl::ColorSourceMultiplier::kOne)
		.setAlphaDestMultiplier(sce::Agc::CxBlendControl::AlphaDestMultiplier::kOne)
		.setColorDestMultiplier(sce::Agc::CxBlendControl::ColorDestMultiplier::kOne);
	frameContext->m_sb.setState(blendControl);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->objectBuffer)
		.setBuffers(1, 1, &bindlessBuffers[128]);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &textureBuffer)
		.setSamplers(0, 1, &defaultSampler)
		.setBuffers(1, 1, &bindlessBuffers[128]);

	mSphereMesh->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));
	uint32_t* objID = static_cast<uint32_t*>(frameContext->m_dcb.allocateTopDown(sizeof(uint32_t), sce::Agc::Alignment::kBuffer));
	for (int i = 0; i < mLights.size(); i++) {

		*objID = i;
		frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs).setUserSrtBuffer(objID, 1);
		DrawBoundMesh(*frameContext, *mSphereMesh);
	}

	sce::Agc::Core::gpuSyncEvent(&frameContext->m_dcb, sce::Agc::Core::SyncWaitMode::kDrainGraphics, sce::Agc::Core::SyncCacheOp::kFlushUncompressedColorBufferForTexture);

	sce::Agc::Core::translate(&bindlessTextures[mDiffLightTex->GetAssetID()], &mDiffLightTarget, sce::Agc::Core::RenderTargetComponent::kData);
	sce::Agc::Core::translate(&bindlessTextures[mSpecLightTex->GetAssetID()], &mSpecLightTarget, sce::Agc::Core::RenderTargetComponent::kData);

	sce::Agc::CxPrimitiveSetup primStateReset;
	primStateReset.init();
	frameContext->m_sb.setState(primStateReset);
}

void GameTechAGCRenderer::CombineBuffers() {
	frameContext->setShaders(nullptr, combineVertexShader->GetAGCPointer(), combinePixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);


	sce::Agc::CxViewport viewPort;
	sce::Agc::Core::setViewport(&viewPort, SCREENWIDTH, SCREENHEIGHT, 0, 0, -1.0f, 1.0f);
	frameContext->m_sb.setState(viewPort);

	sce::Agc::CxRenderTargetMask rtMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0xFF);
	frameContext->m_sb.setState(rtMask);
	frameContext->m_sb.setState(mCombineTarget);

	sce::Agc::CxBlendControl blendControl;
	blendControl.init();
	frameContext->m_sb.setState(blendControl);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs);;

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setSamplers(0, 1, &defaultSampler)
		.setBuffers(0, 1, &textureBuffer);

	quadMesh->BindVertexBuffers(frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs));
	DrawBoundMesh(*frameContext, *quadMesh);

	sce::Agc::Core::gpuSyncEvent(&frameContext->m_dcb, sce::Agc::Core::SyncWaitMode::kDrainGraphics, sce::Agc::Core::SyncCacheOp::kFlushUncompressedColorBufferForTexture);
	sce::Agc::Core::translate(&bindlessTextures[mCombineTex->GetAssetID()], &mCombineTarget, sce::Agc::Core::RenderTargetComponent::kData);
}



void GameTechAGCRenderer::UpdateDebugData() {
	const std::vector<NCL::Debug::DebugStringEntry>& strings = NCL::Debug::GetDebugStrings();
	const std::vector<Debug::DebugLineEntry>& lines = Debug::GetDebugLines();

	currentFrame->textVertCount = 0;
	currentFrame->lineVertCount = 0;

	for (const auto& s : strings) {
		currentFrame->textVertCount += Debug::GetDebugFont()->GetVertexCountForString(s.data);
	}
	currentFrame->lineVertCount = (int)lines.size() * 2;

	currentFrame->debugLinesOffset = currentFrame->data.bytesWritten;

	currentFrame->data.WriteData((void*)lines.data(), (size_t)currentFrame->lineVertCount * LINE_STRIDE);

	currentFrame->debugTextOffset = currentFrame->data.bytesWritten;
	std::vector< NCL::Rendering::SimpleFont::InterleavedTextVertex> verts;

	for (const auto& s : strings) {
		float size = 20.0f;
		Debug::GetDebugFont()->BuildInterleavedVerticesForString(s.data, s.position, s.colour, size, verts);
		//can now copy to GPU visible mem
		size_t count = verts.size() * TEXT_STRIDE;
		memcpy(currentFrame->data.data, verts.data(), count);
		currentFrame->data.data += count;
		currentFrame->data.bytesWritten += count;
		verts.clear();
	}
}

void GameTechAGCRenderer::DisplayRenderPass() {
	sce::Agc::Core::gpuSyncEvent(&frameContext->m_dcb, sce::Agc::Core::SyncWaitMode::kDrainGraphics, sce::Agc::Core::SyncCacheOp::kFlushUncompressedColorBufferForTexture);

	frameContext->setCsShader(gammaCompute->GetAGCPointer());

	sce::Agc::DispatchModifier modifier = gammaCompute->GetAGCPointer()->m_specials->m_dispatchModifier;

	sce::Agc::Core::Texture outputTex; //Alias for our framebuffer tex;
	SceError error = sce::Agc::Core::translate(&outputTex, &backBuffers[currentSwap].renderTarget, sce::Agc::Core::RenderTargetComponent::kData);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kCs)
		.setTextures(0, 1, mCombineTex->GetAGCPointer())
		.setRwTextures(1, 1, &outputTex);
	uint32_t xDims = (outputTex.getWidth() + 7) / 8;
	uint32_t yDims = (outputTex.getHeight() + 7) / 8;

	frameContext->m_dcb.dispatch(xDims, yDims, 1, modifier);
}

void GameTechAGCRenderer::RenderDebugLines() {
	if (currentFrame->lineVertCount == 0) {
		return;
	}
	frameContext->setShaders(nullptr, debugLineVertexShader->GetAGCPointer(), debugLinePixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kLineList);
	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init()
		.setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable)
		.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kDisable);
	frameContext->m_sb.setState(depthControl);

	char* dataPos = currentFrame->data.dataStart + currentFrame->debugLinesOffset;
	size_t dataCount = currentFrame->lineVertCount;

	sce::Agc::Core::BufferSpec bufSpec;
	bufSpec.initAsRegularBuffer(dataPos, LINE_STRIDE, dataCount);
	SceError error = sce::Agc::Core::initialize(&currentFrame->debugLineBuffer, &bufSpec);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->debugLineBuffer);

	frameContext->drawIndexAuto(currentFrame->lineVertCount);
}

void GameTechAGCRenderer::RenderDebugText() {
	if (currentFrame->textVertCount == 0) {
		return;
	}
	frameContext->setShaders(nullptr, debugTextVertexShader->GetAGCPointer(), debugTextPixelShader->GetAGCPointer(), sce::Agc::UcPrimitiveType::Type::kTriList);
	sce::Agc::CxDepthStencilControl depthControl;
	depthControl.init()
		.setDepth(sce::Agc::CxDepthStencilControl::Depth::kDisable)
		.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kDisable);
	frameContext->m_sb.setState(depthControl);

	sce::Agc::CxBlendControl blendControl;
	blendControl.init();
	blendControl.setBlend(sce::Agc::CxBlendControl::Blend::kEnable)
		.setAlphaBlendFunc(sce::Agc::CxBlendControl::AlphaBlendFunc::kAdd)
		.setColorSourceMultiplier(sce::Agc::CxBlendControl::ColorSourceMultiplier::kSrcAlpha)
		.setColorDestMultiplier(sce::Agc::CxBlendControl::ColorDestMultiplier::kOneMinusSrcAlpha)
		.setColorBlendFunc(sce::Agc::CxBlendControl::ColorBlendFunc::kAdd);

	frameContext->m_sb.setState(blendControl);

	char* dataPos = currentFrame->data.dataStart + currentFrame->debugTextOffset;
	size_t dataCount = currentFrame->textVertCount;

	sce::Agc::Core::BufferSpec bufSpec;
	bufSpec.initAsRegularBuffer(dataPos, TEXT_STRIDE, dataCount);
	SceError error = sce::Agc::Core::initialize(&currentFrame->debugTextBuffer, &bufSpec);

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kGs)
		.setConstantBuffers(0, 1, &currentFrame->constantBuffer)
		.setBuffers(0, 1, &currentFrame->debugTextBuffer);

	AGCTexture* debugTex = (AGCTexture*)Debug::GetDebugFont()->GetTexture();

	frameContext->m_bdr.getStage(sce::Agc::ShaderType::kPs)
		.setSamplers(0, 1, &pixelSampler)
		.setTextures(1, 1, debugTex->GetAGCPointer());

	frameContext->drawIndexAuto(currentFrame->textVertCount);
}

void GameTechAGCRenderer::UpdateObjectList() {
	activeObjects.clear();

	char* dataPos = currentFrame->data.data;
	int at = 0;
	const int MAXTEST = 3000;
	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			//if(at > MAXTEST)
			//{
			//	return;
			//}
			if (o->IsActive()) {
				RenderObject* g = o->GetRenderObject();
				if (g) {
					activeObjects.push_back(g);

					ObjectState state;
					state.modelMatrix = g->GetTransform()->GetMatrix();
					state.invModelMatrix = g->GetTransform()->GetMatrix().Inverse();
					state.colour = g->GetColour();
					state.index[0] = 0; //Albedo texture
					state.index[1] = 0; //Normal texture
					state.index[2] = 0; //skinning buffer

					Texture* tex = g->GetAlbedoTexture();
					if (tex) {
						state.index[0] = tex->GetAssetID();
					}

					tex = g->GetNormalTexture();
					if (tex) {
						state.index[1] = tex->GetAssetID();
					}

					if (g->GetMatTextures().size() > 0) {
						for (size_t x = 0; x < g->GetMatTextures().size(); x++) {
						   state.materialLayerAlbedos[x] = g->GetMatTextures()[x * 2];
						   state.materialLayerNormals[x] = g->GetMatTextures()[(x * 2) + 1];
					   }
						state.hasMaterials = 1;
					}
					else {
						state.hasMaterials = 0;
					}

					AGCMesh* m = (AGCMesh*)g->GetMesh();
					if (m && m->GetJointCount() > 0) {//It's a skeleton mesh, need to update transformed vertices buffer

						Buffer* b = g->GetGPUBuffer();
						if (!b) {
							//We've not yet made a buffer to hold the verts of this mesh!
							//We need a new mesh to store the positions, normals, and tangents of this mesh
							size_t vertexSize = sizeof(Vector3) + sizeof(Vector3) + sizeof(Vector4);
							size_t vertexCount = m->GetVertexCount();
							size_t bufferSize = vertexCount * vertexSize;

							char* vertexData = (char*)allocator.Allocate((uint64_t)(bufferSize), sce::Agc::Alignment::kBuffer);

							sce::Agc::Core::BufferSpec bufSpec;
							bufSpec.initAsRegularBuffer(vertexData, vertexSize, vertexCount);

							sce::Agc::Core::Buffer vBuffer;
							SceError error = sce::Agc::Core::initialize(&vBuffer, &bufSpec);

							uint32_t bufferID = bufferCount++;
							b = new AGCBuffer(vBuffer, vertexData);
							b->SetAssetID(bufferID);
							g->SetGPUBuffer(b);

							bindlessBuffers[bufferID] = vBuffer;
						}

						state.index[2] = b->GetAssetID();

						frameJobs.push_back({ g, b->GetAssetID() });
					}

					if (at < MAXTEST) {
						currentFrame->data.WriteData<ObjectState>(state);
						currentFrame->debugLinesOffset += sizeof(ObjectState);
					}
					at++;
				}
			}
		}
	);
	sce::Agc::Core::BufferSpec bufSpec;
	bufSpec.initAsRegularBuffer(dataPos, sizeof(ObjectState), at);
	sce::Agc::Core::initialize(&currentFrame->objectBuffer, &bufSpec);
}

void GameTechAGCRenderer::FillLightUBO() {

	LightData lightData;
	vector<LightData> allData;

	uint32_t bufferID = SKINNED_MESH_COUNT;
	int index = 0;
	for (Light* l : mLights) {
		lightData.lightColour = l->GetColour();
		Light::Type type = l->GetType();
		switch (type) {
		case Light::Direction:
		{
			DirectionLight* dl = static_cast<DirectionLight*> (l);
			lightData.lightDirection = dl->GetDirection();
			lightData.lightPos = dl->GetCentre();
			lightData.lightRadius = dl->GetRadius();
			lightData.lightType = 'd';
		}
		break;
		case Light::Point:
		{
			PointLight* pl = static_cast<PointLight*>(l);
			lightData.lightPos = pl->GetPosition();
			lightData.lightRadius = pl->GetRadius();
			lightData.lightType = 'p';
		}
		break;
		case Light::Spot:
		{
			SpotLight* sl = static_cast<SpotLight*>(l);
			lightData.lightPos = sl->GetPosition();
			lightData.lightDirection = sl->GetDirection();
			lightData.dimDotProd = sl->GetDimProdMin();
			lightData.minDotProd = sl->GetDotProdMin();
			lightData.lightRadius = sl->GetRadius();
			lightData.lightType = 's';
		}
		break;
		default:
			break;
		}

		allData.push_back(lightData);
	}
	size_t lightSize = sizeof(LightData);
	size_t lightCount = allData.size();
	size_t bufferSize = lightSize * lightCount;

	char* lightDataPtr = (char*)allocator.Allocate((uint64_t)(bufferSize), sce::Agc::Alignment::kBuffer);

	sce::Agc::Core::BufferSpec bufSpec;
	bufSpec.initAsRegularBuffer(lightDataPtr, lightSize, lightCount);
	sce::Agc::Core::Buffer lBuffer;
	SceError error = sce::Agc::Core::initialize(&lBuffer, &bufSpec);
	bindlessBuffers[bufferID] = lBuffer;
	memcpy(lightDataPtr, allData.data(), bufferSize);
}
