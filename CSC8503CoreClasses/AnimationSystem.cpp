#ifdef USEGL

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

void AnimationSystem::Update(float dt, vector<GameObject*> updatableObjects) {
	UpdateCurrentFrames(dt);
	UpdateAllAnimationObjects(dt, updatableObjects);
}

void AnimationSystem::UpdateAllAnimationObjects(float dt, vector<GameObject*> updatableObjects) {
	for (GameObject* obj : updatableObjects) {
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

void AnimationSystem::SetGameObjectLists(vector<GameObject*> updatableObjects) {
	for (auto& obj : updatableObjects) {
		if (obj->GetName() == "Guard") {
			mGuardList.emplace_back((GuardObject*)obj);
			AnimationObject* animObj = obj->GetRenderObject()->GetAnimationObject();
			mAnimationList.emplace_back(animObj);
		}
		if (obj->GetName() == "Player") {
			mPlayerList.emplace_back((PlayerObject*)obj);
			AnimationObject* animObj = obj->GetRenderObject()->GetAnimationObject();
			mAnimationList.emplace_back(animObj);
		}
	}
}

void AnimationSystem::SetAnimationState(GameObject* gameObject, GameObject::GameObjectState objState) {
	gameObject->GetRenderObject()->GetAnimationObject()->ReSetCurrentFrame();

	AnimationObject::AnimationType animType = gameObject->GetRenderObject()->GetAnimationObject()->GetAnimationType();
	std::map<GameObject::GameObjectState, std::string>& animMap = animType == AnimationObject::AnimationType::playerAnimation ? 
		mPlayerStateAnimationMap : mGuardStateAnimationMap;

	const std::string& animStr = animMap[objState];
	MeshAnimation* anim = mPreAnimationList[animStr];
	if (animStr == "GuardSprint") {
		gameObject->GetRenderObject()->GetAnimationObject()->SetRate(2.0);
	}
	else {
		gameObject->GetRenderObject()->GetAnimationObject()->SetRate(1.0);
	}
	gameObject->GetRenderObject()->GetAnimationObject()->SetAnimation(anim);
}
#endif

void AnimationSystem::InitGuardStateAnimationMap() {
	mGuardStateAnimationMap = {
	{GameObject::GameObjectState::Idle, "GuardStand"},
	{GameObject::GameObjectState::Walk, "GuardWalk"},
	{GameObject::GameObjectState::Sprint, "GuardSprint"},
	{GameObject::GameObjectState::Point, "GuardPoint"},
	};
	
}

void AnimationSystem::InitPlayerStateAnimationMap() {
	mPlayerStateAnimationMap = {
	{GameObject::GameObjectState::Idle, "PlayerStand"},
	{GameObject::GameObjectState::Walk, "PlayerWalk"},
	{GameObject::GameObjectState::Sprint, "PlayerSprint"},
	};
}
