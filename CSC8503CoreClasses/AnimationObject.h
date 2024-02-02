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
			return material;
		}

		void setMaterial(MeshMaterial* newMaterial) {
			material = newMaterial;
		}

		MeshAnimation* GetMeshAnimation() const {
			return animation;
		}

		void setMeshAnimation(MeshAnimation* newAnimation) {
			animation = newAnimation;
		}

		

		void setMeshAnimationStop(MeshAnimation* newAnimation) {
			animation = newAnimation;

		}
		
		void setMeshAnimationRun();
		
		
		 
	protected:
		Transform			transform;

		CollisionVolume* boundingVolume;
		PhysicsObject* physicsObject;
		RenderObject* renderObject;
		NetworkObject* networkObject;

		bool		isActive;
		int			worldID;
		std::string	name;
		
		Vector3 broadphaseAABB;

		MeshAnimation* animation;
		MeshMaterial* material;
	
	
	};


}