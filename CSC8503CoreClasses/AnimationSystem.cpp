#include "AnimationSystem.h"
#include "Camera.h"
#include "AnimationObject.h"


#define SHADERDIR	"../Assets/Shaders/"
#define MESHDIR		"../Assets/Meshes/"
#define TEXTUREDIR  "../Assets/Textures/"
#define SOUNDSDIR	"../Assets/Sounds/"

AnimationSystem::AnimationSystem(GameWorld& g, std::map<std::string, MeshAnimation*>& preAnimationList) : gameWorld(g),
	mPreAnimationList(preAnimationList)
{
	mShader = nullptr;
	mMesh = nullptr;
	mAnim = nullptr;
	mAnimTexture = nullptr;
	InitGuardStateAnimationMap();
	InitPlayerStateAnimationMap();
}

AnimationSystem::~AnimationSystem() {


}

void AnimationSystem::Clear() {
	mAnimationList.clear();
	mGuardList.clear();
	mPlayerList.clear();
}

void AnimationSystem::Update(float dt, vector<GameObject*> UpdatableObjects) {
	UpdateCurrentFrames(dt);
	UpdateAllAnimationObjects(dt, UpdatableObjects);
}

void AnimationSystem::UpdateAllAnimationObjects(float dt, vector<GameObject*> UpdatableObjects) {
	for (GameObject* obj : UpdatableObjects) {
		if (obj->GetRenderObject()->GetAnimationObject()) {

			AnimationObject* animObj = obj->GetRenderObject()->GetAnimationObject();
			if (animObj != nullptr) {
				int currentFrame = animObj->GetCurrentFrame();
				mMesh = obj->GetRenderObject()->GetMesh();
				mAnim = animObj->GetAnimation();
				mShader = obj->GetRenderObject()->GetShader();

				const Matrix4* invBindPose = mMesh->GetInverseBindPose().data();
				const Matrix4* frameData = mAnim->GetJointData(currentFrame);

				const int* bindPoseIndices = mMesh->GetBindPoseIndices();
				std::vector<std::vector<Matrix4>> frameMatricesVec;
				for (unsigned int i = 0; i < mMesh->GetSubMeshCount(); ++i) {


					Mesh::SubMeshPoses pose;
					mMesh->GetBindPoseState(i, pose);


					vector<Matrix4> frameMatrices;
					for (unsigned int i = 0; i < pose.count; ++i) {
						int jointID = bindPoseIndices[pose.start + i];
						Matrix4 mat = frameData[jointID] * invBindPose[pose.start + i];
						frameMatrices.emplace_back(mat);
					}
					frameMatricesVec.emplace_back(frameMatrices);
				}

				obj->GetRenderObject()->SetCurrentFrame(currentFrame);
				obj->GetRenderObject()->SetFrameMatricesVec(frameMatricesVec);

				frameMatricesVec.clear();
			}
			
		}
	}
}

void AnimationSystem::UpdateCurrentFrames(float dt) {
	for (AnimationObject*& animList : mAnimationList) {
		animList->Update(dt);
	}
}

#ifdef USEGL
void AnimationSystem::PreloadMatTextures(GameTechRenderer& renderer, Mesh& mesh, MeshMaterial& meshMaterial, vector<GLuint>& mMatTextures) {

	for (int i = 0; i < mesh.GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = meshMaterial.GetMaterialForLayer(i);
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		GLuint texID = 0;

		if (filename) {
			string path = *filename;
			std::cout << path << std::endl;
			mAnimTexture = renderer.LoadTexture(path.c_str());
			texID = ((OGLTexture*)mAnimTexture)->GetObjectID();
			std::cout << texID << endl;
		}
		mMatTextures.emplace_back(texID);
	}
}

void AnimationSystem::SetGameObjectLists(vector<GameObject*> UpdatableObjects, vector<GLuint> mPlayerTexture, vector<GLuint>& mGuardTextures) {
	for (auto& obj : UpdatableObjects) {
		if (obj->GetName() == "Guard") {
			mGuardList.emplace_back((GuardObject*)obj);
			AnimationObject* animObj = obj->GetRenderObject()->GetAnimationObject();
			mAnimationList.emplace_back(animObj);
			obj->GetRenderObject()->SetMatTextures(mGuardTextures);

		}
		if (obj->GetName() == "Player") {
			mPlayerList.emplace_back((PlayerObject*)obj);
			AnimationObject* animObj = obj->GetRenderObject()->GetAnimationObject();
			mAnimationList.emplace_back(animObj);
			obj->GetRenderObject()->SetMatTextures(mPlayerTexture);

		}
	}
}
#endif

#ifdef USEPROSPERO
void AnimationSystem::PreloadMatTextures(GameTechAGCRenderer& renderer, Mesh& mesh, MeshMaterial& meshMaterial, vector<sce::Agc::Core::Texture*>& mMatTextures) {

	for (int i = 0; i < mesh.GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = meshMaterial.GetMaterialForLayer(i);
		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		sce::Agc::Core::Texture* texID = nullptr;

		if (filename) {
			string path = *filename;
			std::cout << path << std::endl;
			mAnimTexture = renderer.LoadTexture(path.c_str());
			texID = ((PS5::AGCTexture*)mAnimTexture)->GetAGCPointer();
			std::cout << texID << endl;
		}
		mMatTextures.emplace_back(texID);
	}
}

void AnimationSystem::SetGameObjectLists(vector<GameObject*> UpdatableObjects, vector<sce::Agc::Core::Texture*> mPlayerTexture, vector<sce::Agc::Core::Texture*>& mGuardTextures) {
	for (auto& obj : UpdatableObjects) {
		if (obj->GetName() == "Guard") {
			mGuardList.emplace_back((GuardObject*)obj);
			AnimationObject* animObj = obj->GetRenderObject()->GetAnimationObject();
			mAnimationList.emplace_back(animObj);
			//obj->GetRenderObject()->SetMatTextures(mGuardTextures);

		}
		if (obj->GetName() == "Player") {
			mPlayerList.emplace_back((PlayerObject*)obj);
			AnimationObject* animObj = obj->GetRenderObject()->GetAnimationObject();
			mAnimationList.emplace_back(animObj);
			//obj->GetRenderObject()->SetMatTextures(mPlayerTexture);

		}
	}
}
#endif

void AnimationSystem::SetAnimationState(GameObject* gameObject, GameObject::GameObjectState objState) {
	gameObject->GetRenderObject()->GetAnimationObject()->ReSetCurrentFrame();

	AnimationObject::AnimationType animType = gameObject->GetRenderObject()->GetAnimationObject()->GetAnimationType();
	std::map<GameObject::GameObjectState, std::string>& animMap = animType == AnimationObject::AnimationType::playerAnimation ? 
		mPlayerStateAnimationMap : mGuardStateAnimationMap;

	const std::string& animStr = animMap[objState];

	MeshAnimation* anim = mPreAnimationList[animStr];

	gameObject->GetRenderObject()->GetAnimationObject()->SetAnimation(anim);
}

void AnimationSystem::InitGuardStateAnimationMap() {
	mGuardStateAnimationMap = {
	{GameObject::GameObjectState::Idle, "GuardStand"},
	{GameObject::GameObjectState::Walk, "GuardWalk"},
	{GameObject::GameObjectState::Sprint, "GuardSprint"},
	};
}

void AnimationSystem::InitPlayerStateAnimationMap() {
	mPlayerStateAnimationMap = {
	{GameObject::GameObjectState::Idle, "PlayerStand"},
	{GameObject::GameObjectState::Walk, "PlayerWalk"},
	{GameObject::GameObjectState::Sprint, "PlayerSprint"},
	};
}