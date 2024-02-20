#include "AnimationSystem.h"
#include "Camera.h"
#include "AnimationObject.h"


#define SHADERDIR	"../Assets/Shaders/"
#define MESHDIR		"../Assets/Meshes/"
#define TEXTUREDIR  "../Assets/Textures/"
#define SOUNDSDIR	"../Assets/Sounds/"

AnimationSystem::AnimationSystem(GameWorld& g):gameWorld(g)
{
	mShader = nullptr;
	mMesh = nullptr;
	mAnim = nullptr;
	mGuardState = Stand;
	mPlayerState = Stand;
	
}

AnimationSystem::~AnimationSystem()
{
}

void AnimationSystem::Clear()
{
	mAnimationList.clear();
	mGuardList.clear();
	mPlayerList.clear();
}

void AnimationSystem::Update(float dt, vector<GameObject*> UpdatableObjects,std::map<std::string,MeshAnimation*> preAnimationList)
{
	UpdateCurrentFrames(dt);
	UpdateAnimations(preAnimationList);
	UpdateAllAnimationObjects(dt, UpdatableObjects);
	
}

void AnimationSystem::UpdateAllAnimationObjects(float dt, vector<GameObject*> UpdatableObjects)
{
		for (auto& obj : UpdatableObjects) {
			if (obj->GetAnimationObject()) {
				AnimationObject* animObj = obj->GetAnimationObject();
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
				obj->GetRenderObject()->SetAnimation(obj->GetAnimationObject()->GetAnimation());
				obj->GetRenderObject()->SetMaterial(obj->GetAnimationObject()->GetMaterial());
				obj->GetRenderObject()->SetCurrentFrame(currentFrame);
				obj->GetRenderObject()->SetFrameMatricesVec(frameMatricesVec);
				
				frameMatricesVec.clear();

			}

			
			
		}
	
	
}

void AnimationSystem::UpdateCurrentFrames(float dt)
{
	for ( auto& a : mAnimationList) {
		a->Update(dt);
	}
}


void AnimationSystem::UpdateAnimations(std::map<std::string, MeshAnimation*> preAnimationList)
{
	
	for (auto& obj : mGuardList) {

		GuardObject::GuardState GuardState = obj->GetGuardState();
		
		if (mGuardState != GuardState) {
			mGuardState =(AnimationState) GuardState;
			obj->GetAnimationObject()->ReSetCurrentFrame();

		}
			switch (GuardState)
			{
			case GuardObject::GuardState::Stand:
				obj->GetAnimationObject()->SetAnimation(preAnimationList["GuardStand"]);
				
				break;
			case GuardObject::GuardState::Walk:
				obj->GetAnimationObject()->SetAnimation(preAnimationList["GuardWalk"]);
				
				break;
			case GuardObject::GuardState::Sprint:
				obj->GetAnimationObject()->SetAnimation(preAnimationList["GuardSprint"]);
				
				break;
			}
	}

	for (auto& obj : mPlayerList) {

		PlayerObject::PlayerState PlayerState = obj->GetPlayerState();

		if (mPlayerState != PlayerState) {
			mPlayerState = (AnimationState)PlayerState;
			obj->GetAnimationObject()->ReSetCurrentFrame();

		}
		switch (PlayerState)
		{
		case PlayerObject::PlayerState::Stand:
			obj->GetAnimationObject()->SetAnimation(preAnimationList["PlayerStand"]);

			break;
		case PlayerObject::PlayerState::Walk:
			obj->GetAnimationObject()->SetAnimation(preAnimationList["PlayerWalk"]);

			break;
		case PlayerObject::PlayerState::Sprint:
			obj->GetAnimationObject()->SetAnimation(preAnimationList["PlayerSprint"]);

			break;
		}
	}
	
}

void AnimationSystem::PreloadMatTextures(GameTechRenderer& renderer)
{
	gameWorld.OperateOnContents(
		[&](GameObject* o) {

			if (o->GetAnimationObject()) {
				for (int i = 0; i < o->GetRenderObject()->GetMesh()->GetSubMeshCount(); ++i) {
					const MeshMaterialEntry* matEntry = o->GetAnimationObject()->GetMaterial()->GetMaterialForLayer(i);
					const string* filename = nullptr;
					matEntry->GetEntry("Diffuse", &filename);
					GLuint texID = 0;

					if (filename) {
						string path = *filename;  
						std::cout << path << std::endl;
						mAnimTexture = renderer.LoadTexture(path.c_str());
						texID = ((OGLTexture*)mAnimTexture)->GetObjectID();
						std::cout << texID << endl;
						NCL::Rendering::OGLRenderer::SetTextureRepeating(texID, true);
					}
					
					mMatTextures.emplace_back(texID);
					if (mMatTextures.size() == o->GetRenderObject()->GetMesh()->GetSubMeshCount()) {
						o->GetRenderObject()->SetMatTextures(mMatTextures);	
						mMatTextures.clear();
					}
				
					
				}
				
				
			}

		}
	);
}

void AnimationSystem::SetGameObjectLists(vector<GameObject*> UpdatableObjects) {
	for (auto& obj : UpdatableObjects) {
		
		if (obj->GetName() == "Guard") {
			mGuardList.emplace_back((GuardObject*)obj);
			AnimationObject* animObj = obj->GetAnimationObject();
			mAnimationList.emplace_back(animObj);
		}
		if (obj->GetName() == "Player") {
			mPlayerList.emplace_back((PlayerObject*)obj);
			AnimationObject* animObj = obj->GetAnimationObject();
			mAnimationList.emplace_back(animObj);
		}
	}
}
