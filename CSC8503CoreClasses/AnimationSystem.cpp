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
	animationList.clear();
}

void AnimationSystem::Update(float dt)
{
	
	GetAllAnimationObjects();
	UpdateMaterials();
	UpdateCurrentFrames(dt);


}

void AnimationSystem::GetAllAnimationObjects()
{	animationList.clear();

	gameWorld.OperateOnContents(
		[&](GameObject* o) {
			if (o->GetAnimationObject()) {
				AnimationObject* animObj = o->GetAnimationObject();
				// if (animObj) {
					animationList.emplace_back(animObj);
					
					o->GetRenderObject()->SetAnimation(o->GetAnimationObject()->GetAnimation());
					o->GetRenderObject()->SetMaterial(o->GetAnimationObject()->GetMaterial());
					o->GetRenderObject()->SetCurrentFrame(o->GetAnimationObject()->GetCurrentFrame());
				//std::cout << animObj << std::endl;
				// }
			}
		}
	);
}

void AnimationSystem::UpdateCurrentFrames(float dt)
{
	for ( auto& a : animationList) {
		(*a).Update(dt);
	}
}

void AnimationSystem::UpdateMaterials()
{
	
	
}

void AnimationSystem::UpdateAnimations()
{
	
	for (auto& a : animationList) {
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
		
		
		
	}
	
}

void AnimationSystem::PreloadMatTextures()
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
						texID = NCL::Rendering::OGLTexture::LoadOGLTexture(path);
						std::cout << "++++++++++++++++" << std::endl;
						/*glBindTexture(GL_TEXTURE_2D, texID);*/
						std::cout << "---------------------" << std::endl;
						/*NCL::Rendering::OGLRenderer::SetTextureRepeating(texID, true);*/
						std::cout << "==============================" << std::endl;

						
					}
					
					mMatTextures.emplace_back(texID);
					if (mMatTextures.size() ==4) {
						o->GetRenderObject()->SetMatTextures(mMatTextures);	
					}
				
					
				}
				
				
			}
			//std::cout << o->GetRenderObject()->GetMatTextures().size() << std::endl;

		}
	);
}
