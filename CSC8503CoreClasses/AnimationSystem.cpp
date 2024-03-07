#ifdef USEGL

#include "AnimationSystem.h"
#include "Camera.h"
#include "AnimationObject.h"


#define SHADERDIR	"../Assets/Shaders/"
#define MESHDIR		"../Assets/Meshes/"
#define TEXTUREDIR  "../Assets/Textures/"
#define SOUNDSDIR	"../Assets/Sounds/"

AnimationSystem::AnimationSystem(GameWorld& g):gameWorld(g){
	mShader = nullptr;
	mMesh = nullptr;
	mAnim = nullptr;
	mAnimTexture = nullptr;
	
}

AnimationSystem::~AnimationSystem(){


}

void AnimationSystem::Clear(){
	mAnimationList.clear();
	mGuardList.clear();
	mPlayerList.clear();
}

void AnimationSystem::Update(float dt, vector<GameObject*> UpdatableObjects,std::map<std::string,MeshAnimation*> preAnimationList){
	UpdateCurrentFrames(dt);
	UpdateAnimations(preAnimationList);
	UpdateAllAnimationObjects(dt, UpdatableObjects);
	
}

void AnimationSystem::UpdateAllAnimationObjects(float dt, vector<GameObject*> UpdatableObjects){
	for (vector<GameObject*>::iterator it = UpdatableObjects.begin(); it != UpdatableObjects.end(); ++it) {
		 	GameObject* obj = *it;
			if (obj->GetRenderObject()->GetAnimationObject()) {
				AnimationObject* animObj = obj->GetRenderObject()->GetAnimationObject();
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

void AnimationSystem::UpdateCurrentFrames(float dt){
	
	for (vector<AnimationObject*>::iterator it = mAnimationList.begin(); it != mAnimationList.end(); ++it) {
		AnimationObject* animList = *it;
		animList->Update(dt);
	}

}


void AnimationSystem::UpdateAnimations(std::map<std::string, MeshAnimation*> preAnimationList){
	for (vector<GuardObject*>::iterator it = mGuardList.begin(); it != mGuardList.end(); ++it) {
		GuardObject* obj = *it;

		GameObject::GameObjectState mObjectState = obj->GetGameOjbectState();
		
		
		if (mGuardState != mObjectState) {
			mGuardState =(GameObject::GameObjectState)mObjectState;
			obj->GetRenderObject()->GetAnimationObject()->ReSetCurrentFrame();

			switch (mObjectState)
			{
			case GameObject::GameObjectState::Idle:
				obj->GetRenderObject()->GetAnimationObject()->SetAnimation(preAnimationList["GuardStand"]);
				obj->GetRenderObject()->GetAnimationObject()->SetRate(1.0);
				break;
			case GameObject::GameObjectState::Walk:
				obj->GetRenderObject()->GetAnimationObject()->SetAnimation(preAnimationList["GuardWalk"]);
				obj->GetRenderObject()->GetAnimationObject()->SetRate(1.0);
				break;
			case GameObject::GameObjectState::Sprint:
				obj->GetRenderObject()->GetAnimationObject()->SetAnimation(preAnimationList["GuardSprint"]);
				obj->GetRenderObject()->GetAnimationObject()->SetRate(2.0);
				break;
			}

		}
			
	}

	for (vector<PlayerObject*>::iterator it = mPlayerList.begin(); it != mPlayerList.end(); ++it) {
		PlayerObject* obj = *it;
		GameObject::GameObjectState mObjectState = obj->GetGameOjbectState();

		if (mPlayerState != mObjectState) {
			mPlayerState = (GameObject::GameObjectState)mObjectState;
			obj->GetRenderObject()->GetAnimationObject()->ReSetCurrentFrame();

			switch (mObjectState)
			{
			case GameObject::GameObjectState::Idle:
				obj->GetRenderObject()->GetAnimationObject()->SetAnimation(preAnimationList["PlayerStand"]);

				break;
			case GameObject::GameObjectState::Walk:
				obj->GetRenderObject()->GetAnimationObject()->SetAnimation(preAnimationList["PlayerWalk"]);

				break;
			case GameObject::GameObjectState::Sprint:
				obj->GetRenderObject()->GetAnimationObject()->SetAnimation(preAnimationList["PlayerSprint"]);

				break;
			}
		}
		
	}
	
}

void AnimationSystem::PreloadMatTextures(GameTechRenderer& renderer, Mesh& mesh, MeshMaterial& meshMaterial, vector<GLuint>& mMatTextures){
	
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
	for (vector<GameObject*>::iterator it = UpdatableObjects.begin(); it != UpdatableObjects.end(); ++it) {
		GameObject* obj = *it;
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