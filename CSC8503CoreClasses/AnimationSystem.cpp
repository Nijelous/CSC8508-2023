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

void AnimationSystem::Update(float dt,std::map<std::string,MeshAnimation*> preAnimationList)
{
	
	UpdateAllAnimationObjects(dt);
	UpdateMaterials();
	
}

void AnimationSystem::UpdateAllAnimationObjects(float dt)
{
	mAnimationList.clear();
	
	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			if (o->GetAnimationObject()) {
				AnimationObject* animObj = o->GetAnimationObject();
				mAnimationList.emplace_back(animObj);
				//TODO may it is not a good position to run
				UpdateCurrentFrames(dt);

				mMesh = o->GetRenderObject()->GetMesh();
				mAnim = animObj->GetAnimation();
				mShader = o->GetRenderObject()->GetShader();
				int currentFrame = animObj->GetCurrentFrame();

				const Matrix4* bindPose = mMesh->GetBindPose().data();
				const Matrix4* invBindPose = mMesh->GetInverseBindPose().data();
				const Matrix4* frameData = mAnim->GetJointData(currentFrame);
				vector<Matrix4> frameMatrices;

				
				
				for (unsigned int a = 0; a < mMesh->GetJointCount(); ++a) {
					frameMatrices.emplace_back(frameData[a] * invBindPose[a]);
				}
				
				o->GetRenderObject()->SetAnimation(o->GetAnimationObject()->GetAnimation());
				o->GetRenderObject()->SetMaterial(o->GetAnimationObject()->GetMaterial());
				o->GetRenderObject()->SetCurrentFrame(o->GetAnimationObject()->GetCurrentFrame());
				o->GetRenderObject()->SetFrameMatrices(frameMatrices);
			}
		}
	);
}

void AnimationSystem::UpdateCurrentFrames(float dt)
{
	for ( auto& a : mAnimationList) {
		(*a).Update(dt);
	}
}

void AnimationSystem::UpdateMaterials()
{
	
	
}

void AnimationSystem::UpdateAnimations(std::map<std::string, MeshAnimation*> preAnimationList)
{
	
	/*for (auto& a : mAnimationList) {
		AnimationObject::mAnimationState state = (*a).GetAnimationState();
		switch (state)
		{
		case AnimationObject::mAnimationState::stand:
			break;
		case AnimationObject::mAnimationState::run:
			break;
		case AnimationObject::mAnimationState::jumpUp:
			break;
		case AnimationObject::mAnimationState::jumpDown:
			break;
		}
		
		
		
	}*/
	for (auto& a : mAnimationList) {
		//a->SetAnimation(mGuardAnimationHappy)
	}
	
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
						NCL::Rendering::OGLRenderer::SetTextureRepeating(texID, true);
					}
					
					mMatTextures.emplace_back(texID);
					if (mMatTextures.size() ==4) {
						o->GetRenderObject()->SetMatTextures(mMatTextures);	
					}
				
					
				}
				
				
			}

		}
	);
}
