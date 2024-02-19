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
}

void AnimationSystem::Update(float dt, vector<GameObject*> UpdatableObjects,std::map<std::string,MeshAnimation*> preAnimationList)
{
	UpdateCurrentFrames(dt);
	UpdateAnimations(preAnimationList);
	UpdateAllAnimationObjects(dt, UpdatableObjects);
	/*UpdateMaterials();*/
	
}

void AnimationSystem::UpdateAllAnimationObjects(float dt, vector<GameObject*> UpdatableObjects)
{
	mAnimationList.clear();
	mGuardList.clear();
	mPlayerList.clear();
		for (auto& o : UpdatableObjects) {
			//Animation List
			
			if (o->GetAnimationObject()) {
				AnimationObject* animObj = o->GetAnimationObject();
				mAnimationList.emplace_back(animObj);
				//TODO may it is not a good position to run
				
				int currentFrame = animObj->GetCurrentFrame();
				mMesh = o->GetRenderObject()->GetMesh();
				mAnim = animObj->GetAnimation();
				mShader = o->GetRenderObject()->GetShader();
				
				const Matrix4* invBindPose = mMesh->GetInverseBindPose().data();
				const Matrix4* frameData = mAnim->GetJointData(currentFrame);
				
				const int* bindPoseIndices = mMesh->GetBindPoseIndices();
				std::vector<std::vector<Matrix4>> frameMatricesVec;
				for (unsigned int i = 0; i < mMesh->GetSubMeshCount(); ++i) {

					
					Mesh::SubMeshPoses pose;
					mMesh->GetBindPoseState(i, pose);
					

					vector<Matrix4> frameMatrices;
					for (unsigned int i = 0; i < pose.count; ++i) {
						/*
						We can now grab the correct matrix for a given pose.
						Each matrix is relative to a given joint on the original mesh.
						We can perform the lookup for this by grabbing a set of indices
						from the mesh.
						*/
						int jointID = bindPoseIndices[pose.start + i];

						Matrix4 mat = frameData[jointID] * invBindPose[pose.start + i];

						frameMatrices.emplace_back(mat);
					}
					frameMatricesVec.emplace_back(frameMatrices);
				}
				o->GetRenderObject()->SetAnimation(o->GetAnimationObject()->GetAnimation());
				o->GetRenderObject()->SetMaterial(o->GetAnimationObject()->GetMaterial());
				o->GetRenderObject()->SetCurrentFrame(currentFrame);
				o->GetRenderObject()->SetFrameMatricesVec(frameMatricesVec);
				
				frameMatricesVec.clear();
				
				
				

				if (o->GetName() == "Guard") {
					mGuardList.emplace_back((GuardObject*)o);
					/*std::cout << "find the guard" << std::endl;*/
				}
				if (o->GetName() == "Player") {
					mPlayerList.emplace_back((PlayerObject*)o);
					/*std::cout << "find the player" << std::endl;*/
				}
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
	
	for (auto& a : mGuardList) {

		GuardObject::GuardState GuardState = a->GetGuardState();
		
		if (mGuardState != GuardState) {
			mGuardState =(AnimationState) GuardState;
			a->GetAnimationObject()->ReSetCurrentFrame();

		}
			switch (GuardState)
			{
			case GuardObject::GuardState::Stand:
				//std::cout << (*a).GetGuardState() << std::endl;
				a->GetAnimationObject()->SetAnimation(preAnimationList["GuardStand"]);
				
				break;
			case GuardObject::GuardState::Walk:
				//std::cout << (*a).GetGuardState() << std::endl;
				a->GetAnimationObject()->SetAnimation(preAnimationList["GuardWalk"]);
				
				break;
			case GuardObject::GuardState::Sprint:
				//std::cout << (*a).GetGuardState() << std::endl;
				a->GetAnimationObject()->SetAnimation(preAnimationList["GuardSprint"]);
				
				break;
			}
	}

	for (auto& a : mPlayerList) {

		PlayerObject::PlayerState PlayerState = a->GetPlayerState();

		if (mPlayerState != PlayerState) {
			mPlayerState = (AnimationState)PlayerState;
			a->GetAnimationObject()->ReSetCurrentFrame();

		}
		switch (PlayerState)
		{
		case PlayerObject::PlayerState::Stand:
			//std::cout << (*a).GetPlayerState()<< std::endl;
			a->GetAnimationObject()->SetAnimation(preAnimationList["PlayerStand"]);

			break;
		case PlayerObject::PlayerState::Walk:
			//std::cout << (*a).GetPlayerState() << std::endl;
			a->GetAnimationObject()->SetAnimation(preAnimationList["PlayerWalk"]);

			break;
		case PlayerObject::PlayerState::Sprint:
			//std::cout << (*a).GetPlayerState()<< std::endl;
			a->GetAnimationObject()->SetAnimation(preAnimationList["PlayerSprint"]);

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
