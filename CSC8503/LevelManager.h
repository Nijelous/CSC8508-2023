#pragma once
#include "Level.h"
#include "GameTechRenderer.h"
#include "PhysicsSystem.h"
#include "AnimationSystem.h"
#include "InventoryBuffSystem/InventoryBuffSystem.h"

using namespace NCL::Maths;
using namespace InventoryBuffSystem;

namespace NCL {
	constexpr float PLAYER_MESH_SIZE = 3.0f;
	constexpr float PLAYER_INVERSE_MASS = 0.5f;
	namespace CSC8503 {
		class PlayerObject;
		class GuardObject;
		class RecastBuilder;
		class Helipad;
		class FlagGameObject;
		class PickupGameObject;
		class LevelManager {
		public:
			LevelManager();
			~LevelManager();
			std::vector<Level*> GetLevels() { return mLevelList; }
			std::vector<Room*> GetRooms() { return mRoomList; }
			Level* GetActiveLevel() const { return mLevelList[mActiveLevel]; }
			Vector3 GetPlayerStartPosition(int player) const { return (*mLevelList[mActiveLevel]).GetPlayerStartTransform(player).GetPosition(); }
			void LoadLevel(int levelID, int playerID);

			PlayerObject* GetTempPlayer() { return mTempPlayer; }

			void SetTempPlayer(PlayerObject* playerObject) { mTempPlayer = playerObject; }

			GameWorld* GetGameWorld() { return mWorld; }

			PhysicsSystem* GetPhysics() { return mPhysics; }

			GameTechRenderer* GetRenderer() { return mRenderer; }

			virtual void Update(float dt, bool isUpdatingObjects);

			void CreatePlayerObjectComponents(PlayerObject& playerObject, const Vector3& position) const;

			void CreatePlayerObjectComponents(PlayerObject& playerObject, const Transform& playerTransform);

			bool CheckGameWon();

		protected:
			virtual void InitialiseAssets();

			void LoadMap(const std::map<Vector3, TileType>& tileMap, const Vector3& startPosition);

			void LoadLights(const std::vector<Light*>& lights, const Vector3& centre);

			void LoadGuards(int guardCount);

			void LoadItems(const std::vector<Vector3> itemPositions);

			GameObject* AddWallToWorld(const Vector3& position);
			GameObject* AddFloorToWorld(const Vector3& position);
			Helipad* AddHelipadToWorld(const Vector3& position);

			FlagGameObject* AddFlagToWorld(const Vector3& position, InventoryBuffSystemClass* inventoryBuffSystemClassPtr);

			PickupGameObject* AddPickupToWorld(const Vector3& position, InventoryBuffSystemClass* inventoryBuffSystemClassPtr);

			PlayerObject* AddPlayerToWorld(const Transform& transform, const std::string& playerName);

			GuardObject* AddGuardToWorld(const Vector3& position, const std::string& guardName);

			std::vector<Level*> mLevelList;
			std::vector<Room*> mRoomList;
			std::vector<GameObject*> mLevelLayout;

			RecastBuilder* mBuilder;
			GameTechRenderer* mRenderer;
			GameWorld* mWorld;
			PhysicsSystem* mPhysics;
			AnimationSystem* mAnimation;

			vector<GameObject*> mUpdatableObjects;

			// meshes
			Mesh* mCubeMesh;
			Mesh* mSphereMesh;
			Mesh* mCapsuleMesh;
			Mesh* mCharMesh;
			Mesh* mEnemyMesh;
			Mesh* mBonusMesh;

			// textures
			Texture* mBasicTex;
			Texture* mKeeperAlbedo;
			Texture* mKeeperNormal;
			Texture* mFloorAlbedo;
			Texture* mFloorNormal;

			// shaders
			Shader* mBasicShader;
			Shader* mSoldierShader;

			// animated meshes
			Mesh* mSoldierMesh;
			MeshAnimation* mSoldierAnimation;
			MeshMaterial* mSoldierMaterial;

			Helipad* mHelipad;

			PlayerObject* mTempPlayer;

			InventoryBuffSystemClass* mInventoryBuffSystemClassPtr = new InventoryBuffSystemClass();

			int mActiveLevel;
		};
	}
}

