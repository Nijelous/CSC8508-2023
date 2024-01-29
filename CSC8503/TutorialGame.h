#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			virtual void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			virtual void InitWorld();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitOBBAABB();
			void BridgeConstraintTest(Vector3 startPosition);

			void InitDefaultFloor();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			GameObject* AddFloorToWorld(const Vector3& position, const std::string& objectName);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, bool applyPhysicsfloat, float inverseMass = 10.0f, const std::string& objectName = "");
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f, const std::string& objectName = "");
			GameObject* AddOBBCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, const std::string& objectName = "");
			GameObject* AddAABBCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, const std::string& objectName = "");

			GameObject* AddPlayerToWorld(const Vector3& position, const std::string& objectName);
			GameObject* AddEnemyToWorld(const Vector3& position, const std::string& objectName);
			GameObject* AddBonusToWorld(const Vector3& position, const std::string& objectName);

			StateGameObject* AddStateObjectToWorld(const Vector3& position, const std::string& objectName);
			StateGameObject* testStateObject;

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			KeyboardMouseController controller;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			Mesh*	capsuleMesh = nullptr;
			Mesh*	cubeMesh	= nullptr;
			Mesh*	sphereMesh	= nullptr;

			Texture*	basicTex	= nullptr;
			Shader*		basicShader = nullptr;

			//Coursework Meshes
			Mesh*	charMesh	= nullptr;
			Mesh*	enemyMesh	= nullptr;
			Mesh*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 3, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;
		};
	}
}

