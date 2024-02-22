#pragma once
#include "Level.h"
#include "GameTechRenderer.h"
#include "PhysicsSystem.h"
#include "AnimationSystem.h"
#include "InventoryBuffSystem/InventoryBuffSystem.h"
#include "InventoryBuffSystem/PlayerInventory.h"
#include "SuspicionSystem/SuspicionSystem.h"

using namespace NCL::Maths;
using namespace InventoryBuffSystem;
using namespace SuspicionSystem;

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
		class SoundEmitter;

		struct GameResults {
			bool mGameWon;
			int mCurrentPoints;

			GameResults(bool gameWon, int currentPoints) {
				mGameWon = gameWon;
				mCurrentPoints = currentPoints;
			}
		};

		class LevelManager : PlayerInventoryObserver {
		public:
			static LevelManager* GetLevelManager();
			void ResetLevel();
			void ClearLevel();
			std::vector<Level*> GetLevels() { return mLevelList; }
			std::vector<Room*> GetRooms() { return mRoomList; }
			Level* GetActiveLevel() const { return mLevelList[mActiveLevel]; }

			Vector3 GetPlayerStartPosition(int player) const { return (*mLevelList[mActiveLevel]).GetPlayerStartTransform(player).GetPosition(); }
			void LoadLevel(int levelID, int playerID,  bool isMultiplayer = false);
			PlayerObject* GetTempPlayer() { return mTempPlayer; }

			void SetTempPlayer(PlayerObject* playerObject) { mTempPlayer = playerObject; }

			GameWorld* GetGameWorld() { return mWorld; }

			PhysicsSystem* GetPhysics() { return mPhysics; }

			GameTechRenderer* GetRenderer() { return mRenderer; }

			virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo) override;

			const std::vector<Matrix4>& GetLevelMatrices() { return mLevelMatrices; }

			virtual void Update(float dt, bool isUpdatingObjects, bool isPaused);

			void CreatePlayerObjectComponents(PlayerObject& playerObject, const Vector3& position) const;

			void AddUpdateableGameObject(GameObject& object);

			void CreatePlayerObjectComponents(PlayerObject& playerObject, const Transform& playerTransform);

			GameResults CheckGameWon();
			bool CheckGameLost();
		
		protected:
			LevelManager();
			~LevelManager();

			static LevelManager* instance;

			virtual void InitialiseAssets();

			void InitialiseIcons();

			void LoadMap(const std::map<Vector3, TileType>& tileMap, const Vector3& startPosition);

			void LoadLights(const std::vector<Light*>& lights, const Vector3& centre);

			void LoadGuards(int guardCount);

			void LoadItems(const std::vector<Vector3>& itemPositions);

			void LoadVents(const std::vector<Vent*>& vents, const std::vector<int> ventConnections);

			void LoadDoors(const std::vector<Door*>& doors, const Vector3& centre);
			void SendWallFloorInstancesToGPU();

			GameObject* AddWallToWorld(const Vector3& position);
			GameObject* AddFloorToWorld(const Vector3& position);
			Helipad* AddHelipadToWorld(const Vector3& position);
			Vent* AddVentToWorld(Vent* vent);
			Door* AddDoorToWorld(Door* door, const Vector3& offset);
			PrisonDoor* AddPrisonDoorToWorld(PrisonDoor* door);

			FlagGameObject* AddFlagToWorld(const Vector3& position, InventoryBuffSystemClass* inventoryBuffSystemClassPtr);

			PickupGameObject* AddPickupToWorld(const Vector3& position, InventoryBuffSystemClass* inventoryBuffSystemClassPtr);

			PlayerObject* AddPlayerToWorld(const Transform& transform, const std::string& playerName);

			GuardObject* AddGuardToWorld(const vector<Vector3> nodes, const Vector3 prisonPosition, const std::string& guardName);

			SoundEmitter* AddSoundEmitterToWorld(const Vector3& position, LocationBasedSuspicion* locationBasedSuspicionPTR);

			std::vector<Level*> mLevelList;
			std::vector<Room*> mRoomList;
			std::vector<GameObject*> mLevelLayout;
			std::vector<Matrix4> mLevelMatrices;

			RecastBuilder* mBuilder;
			GameTechRenderer* mRenderer;
			GameWorld* mWorld;
			PhysicsSystem* mPhysics;
			AnimationSystem* mAnimation;

			vector<GameObject*> mUpdatableObjects;

			// meshes
			Mesh* mCubeMesh;
			Mesh* mWallFloorCubeMesh;
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

			//icons
			UI* mUi;
			Texture* mInventorySlotTex;

			Texture* mHighlightAwardTex;
			Texture* mLightOffTex;
			Texture* mMakingNoiseTex;
			Texture* mSilentRunTex;
			Texture* mSlowDownTex;
			Texture* mStunTex;
			Texture* mSwapPositionTex;

			Texture* mSuspensionBarTex;
			Texture* mSuspensionIndicatorTex;

			// shaders
			Shader* mBasicShader;

			// animation 
			Mesh* mGuardMesh;
			Mesh* mPlayerMesh;
			Mesh* mRigMesh;
			MeshMaterial* mRigMaterial;
			MeshMaterial* mGuardMaterial;
			MeshMaterial* mPlayerMaterial;

			Shader* mAnimationShader;
			Shader* mAnimationShader2;

			//animation guard
			std::map<std::string, MeshAnimation*> mPreAnimationList;
			MeshAnimation* mGuardAnimationStand;
			MeshAnimation* mGuardAnimationSprint;
			MeshAnimation* mGuardAnimationWalk;
			MeshAnimation* mGuardAnimationHappy;
			MeshAnimation* mGuardAnimationAngry;

			MeshAnimation* mPlayerAnimationStand;
			MeshAnimation* mPlayerAnimationSprint;
			MeshAnimation* mPlayerAnimationWalk;

			MeshAnimation* mRigAnimationStand;
			MeshAnimation* mRigAnimationSprint;
			MeshAnimation* mRigAnimationWalk;
			
			

			// game objects
			Helipad* mHelipad;
			PlayerObject* mTempPlayer;

			InventoryBuffSystemClass* mInventoryBuffSystemClassPtr = nullptr; 
			SuspicionSystemClass* mSuspicionSystemClassPtr = nullptr;

			// key variables
			int mActiveLevel;
			float mTimer;
		};
	}
}

