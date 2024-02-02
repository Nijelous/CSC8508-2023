#include "GameObject.h"
#include "Transform.h"
#include "CollisionVolume.h"
#include "MeshAnimation.h"
#include "../NCLCoreClasses/Camera.h"
#include "../OpenGLRendering/OGLRenderer.h"



using std::vector;
class NetworkObject;
class RenderObject;
class PhysicsObject;
class MeshAnimation;
class MeshMaterial;
namespace NCL::CSC8503 {
	
	class AnimationObject : public GameObject {
	public:
		AnimationObject(const std::string& name = "");
		~AnimationObject();

		MeshMaterial* GetMeshMaterial() const {
			return mMaterial;
		}

		void setMaterial(MeshMaterial* newMaterial) {
			mMaterial = newMaterial;
		}

		MeshAnimation* GetMeshAnimation() const {
			return mAnimation;
		}

		void setMeshAnimation(MeshAnimation* newAnimation) {
			mAnimation = newAnimation;
		}

		

		void setMeshAnimationStop(MeshAnimation* newAnimation) {
			mAnimation = newAnimation;

		}
		
		void setMeshAnimationRun();
		
		
		 
	protected:
		Transform			mTransform;

		CollisionVolume* mBoundingVolume;
		PhysicsObject* mPhysicsObject;
		RenderObject* mRenderObject;
		NetworkObject* mNetworkObject;

		bool		isActive;
		int			worldID;
		std::string	mName;
		
		Vector3 broadphaseAABB;

		MeshAnimation* mAnimation;
		MeshMaterial* mMaterial;
	
	
	};


}