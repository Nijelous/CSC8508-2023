#include "AnimationSystem.h"
#include "Camera.h"
#include "AnimationObject.h"


#define SHADERDIR	"../Assets/Shaders/"
#define MESHDIR		"../Assets/Meshes/"
#define TEXTUREDIR  "../Assets/Textures/"
#define SOUNDSDIR	"../Assets/Sounds/"

AnimationSystem::AnimationSystem(GameWorld& g):gameWorld(g)
{
	
	
	
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
	UpdateAnimations(preAnimationList);
	UpdateAllAnimationObjects(dt, UpdatableObjects);
	/*UpdateMaterials();*/
	UpdateCurrentFrames(dt);
}

void AnimationSystem::UpdateAllAnimationObjects(float dt, vector<GameObject*> UpdatableObjects)
{
	mAnimationList.clear();
	mGuardList.clear();
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
					std::cout << "find the guard" << std::endl;
				}
				if (o->GetName() == "Player") {
					
					std::cout << "find the player" << std::endl;
				}
			}

			
			
		}
	
	
}

void AnimationSystem::UpdateCurrentFrames(float dt)
{
	for ( auto& a : mAnimationList) {
		(*a).Update(dt);
	}
}


void AnimationSystem::UpdateAnimations(std::map<std::string, MeshAnimation*> preAnimationList)
{
	
	//for (auto& a : mGuardList) {
	//	GuardObject::GuardState state = (*a).GetGuardState();
	//	
	//	switch (state)
	//	{
	//	case AnimationObject::mAnimationState::Stand:
	//		//std::cout << (*a).GetGuardState() << std::endl;
	//		a->GetAnimationObject()->SetAnimation(preAnimationList["GStand"]);
	//		
	//		break;
	//	case AnimationObject::mAnimationState::Walk:
	//		//std::cout << (*a).GetGuardState() << std::endl;
	//		a->GetAnimationObject()->SetAnimation(preAnimationList["GWalk"]);
	//		break;
	//	case AnimationObject::mAnimationState::Sprint:
	//		//std::cout << (*a).GetGuardState() << std::endl;
	//		a->GetAnimationObject()->SetAnimation(preAnimationList["GSprint"]);
	//		break;
	//	case AnimationObject::mAnimationState::Happy:
	//		//std::cout << (*a).GetGuardState() << std::endl;
	//		a->GetAnimationObject()->SetAnimation(preAnimationList["GHappy"]);
	//		break;
	//	}
	//	
	//	
	//	
	//}
	
}

void AnimationSystem::PreloadMatTextures(GameTechRenderer* renderer)
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
						mAnimTexture = renderer->LoadTexture(path.c_str());
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
