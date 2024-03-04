#pragma once
#include "Level.h"
#include "GameTechRenderer.h"
#include "PhysicsSystem.h"
#include "AnimationSystem.h"
#include "InventoryBuffSystem/InventoryBuffSystem.h"
#include "InventoryBuffSystem/PlayerInventory.h"
#include "SuspicionSystem/SuspicionSystem.h"
#include "SoundManager.h"

using namespace NCL::Maths;
using namespace InventoryBuffSystem;
using namespace SuspicionSystem;

namespace NCL {
	constexpr float PLAYER_MESH_SIZE = 3.0f;
	constexpr float PLAYER_INVERSE_MASS = 0.5f;
	constexpr float TIME_UNTIL_FIXED_UPDATE = 0.25f;
	constexpr float INIT_TIMER_VALUE = 1000;
	namespace CSC8503 {
		class PlayerObject;
		class GuardObject;
		class RecastBuilder;
		class Helipad;
		class CCTV;
		class FlagGameObject;
		class PickupGameObject;
		class SoundEmitter;
		class InteractableDoor;
		class PointGameObject;
		class NetworkPlayer;
		class InventoryBuffSystem::PlayerInventoryObserver;
		class InventoryBuffSystem::PlayerBuffsObserver;
		struct GameResults {
			bool mGameWon;
			int mCurrentPoints;

			GameResults(bool gameWon, int currentPoints) {
				mGameWon = gameWon;
				mCurrentPoints = currentPoints;
			}
		};

		enum GameStates {
			MenuState,
			LevelState,
			PauseState
		};

		enum PolyFlags {
			FloorFlag = 1,
			ClosedDoorFlag = 2,
			LockedDoorFlag = 4,
			MAX_FLAGS = 8
		};

		class LevelManager : public PlayerInventoryObserver {
		public:
			static LevelManager* GetLevelManager();
			void ResetLevel();
			void ClearLevel();
			GameStates GetGameState() { return mGameState; }
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

			RecastBuilder* GetBuilder() { return mBuilder; }

			InventoryBuffSystemClass* GetInventoryBuffSystem();

			SuspicionSystemClass* GetSuspicionSystem();

			UISystem* GetUiSystem() { return mUi; };
			SoundManager* GetSoundManager() { return mSoundManager; };

			virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved = false) override;

			const std::vector<Matrix4>& GetLevelFloorMatrices() { return mLevelFloorMatrices; }

			const std::vector<Matrix4>& GetLevelWallMatrices() { return mLevelWallMatrices; }

			const std::vector<Matrix4>& GetLevelCornerWallMatrices() { return mLevelCornerWallMatrices; }

			virtual void Update(float dt, bool isUpdatingObjects, bool isPaused);

			void FixedUpdate(float dt);

			void CreatePlayerObjectComponents(PlayerObject& playerObject, const Vector3& position) const;

			void AddUpdateableGameObject(GameObject& object);

			void CreatePlayerObjectComponents(PlayerObject& playerObject, const Transform& playerTransform);

			void ChangeEquippedIconTexture(int itemSlot, PlayerInventory::item equippedItem);

			void DropEquippedIconTexture(int itemSlot);

			void ResetEquippedIconTexture();

			GameResults CheckGameWon();

			bool CheckGameLost();

			std::vector<GuardObject*>& GetGuardObjects();

			void AddBuffToGuards(PlayerBuffs::buff buffToApply);

			FlagGameObject* GetMainFlag();

			void LoadDoorInNavGrid(float* position, float* halfSize, PolyFlags flag);
		protected:
			LevelManager();
			~LevelManager();

			static LevelManager* instance;

			virtual void InitialiseAssets();

			void InitialiseIcons();

			void LoadMap(const std::unordered_map<Transform, TileType>& tileMap, const Vector3& startPosition);

			void LoadLights(const std::vector<Light*>& lights, const Vector3& centre);

			void LoadGuards(int guardCount);

			void LoadItems(const std::vector<Vector3>& itemPositions, const std::vector<Vector3>& roomItemPositions, const bool& isMultiplayer);

			void LoadVents(const std::vector<Vent*>& vents, const std::vector<int> ventConnections);

			void LoadDoors(const std::vector<Door*>& doors, const Vector3& centre);

			void LoadCCTVs(const std::vector<Transform>& transforms, const Vector3& startPosition);

			void LoadDoorsInNavGrid();

			void SendWallFloorInstancesToGPU();

			GameObject* AddWallToWorld(const Transform& transform);
			GameObject* AddCornerWallToWorld(const Transform& transform);
			GameObject* AddFloorToWorld(const Transform& transform);
			CCTV* AddCCTVToWorld(const Transform& transform);
			Helipad* AddHelipadToWorld(const Vector3& position);
			Vent* AddVentToWorld(Vent* vent);
			InteractableDoor* AddDoorToWorld(Door* door, const Vector3& offset);
			PrisonDoor* AddPrisonDoorToWorld(PrisonDoor* door);

			FlagGameObject* AddFlagToWorld(const Vector3& position, InventoryBuffSystemClass* inventoryBuffSystemClassPtr, SuspicionSystemClass* suspicionSystemClassPtr);

			PickupGameObject* AddPickupToWorld(const Vector3& position, InventoryBuffSystemClass* inventoryBuffSystemClassPtr, const bool& isMultiplayer);

			PointGameObject* AddPointObjectToWorld(const Vector3& position, int pointsWorth = 5, float initCooldown = 10);

			PlayerObject* AddPlayerToWorld(const Transform& transform, const std::string& playerName, PrisonDoor* mPrisonDoor);

			GuardObject* AddGuardToWorld(const vector<Vector3> nodes, const Vector3 prisonPosition, const std::string& guardName);

			SoundEmitter* AddSoundEmitterToWorld(const Vector3& position, LocationBasedSuspicion* locationBasedSuspicionPTR);

			std::vector<Level*> mLevelList;
			std::vector<Room*> mRoomList;
			std::vector<GameObject*> mLevelLayout;
			std::vector<Matrix4> mLevelFloorMatrices;
			std::vector<Matrix4> mLevelWallMatrices;
			std::vector<Matrix4> mLevelCornerWallMatrices;
			GameObject* mBaseFloor;
			GameObject* mBaseWall;
			GameObject* mBaseCornerWall;

			RecastBuilder* mBuilder;
			GameTechRenderer* mRenderer;
			GameWorld* mWorld;
			PhysicsSystem* mPhysics;
			AnimationSystem* mAnimation;

			SoundManager* mSoundManager;

			vector<GameObject*> mUpdatableObjects;

			// meshes
			Mesh* mCubeMesh;
			Mesh* mFloorCubeMesh;
			Mesh* mSphereMesh;
			Mesh* mCapsuleMesh;
			Mesh* mCharMesh;
			Mesh* mEnemyMesh;
			Mesh* mBonusMesh;
			Mesh* mStraightWallMesh;
			Mesh* mCornerWallMesh;
			Mesh* mCCTVMesh;

			// textures
			Texture* mBasicTex;
			Texture* mKeeperAlbedo;
			Texture* mKeeperNormal;
			Texture* mFloorAlbedo;
			Texture* mFloorNormal;
			Texture* mWallTex;
			Texture* mWallNormal;

			UISystem* mUi;
			Texture* mInventorySlotTex;
			Texture* mCrossTex;

			//powerup Icon

			Texture* mSilentRunTex;
			Texture* mSpeedUpTex;
			Texture* mSlowDownTex;
			Texture* mStunTex;


			Texture* mLowSuspisionBarTex;
			Texture* mMidSuspisionBarTex;
			Texture* mHighSuspisionBarTex;
			Texture* mSuspisionIndicatorTex;

			FlagGameObject* mMainFlag;
			//item icon
			Texture* mFlagIconTex;
			Texture* mKeyIconTex1;
			Texture* mKeyIconTex2;
			Texture* mKeyIconTex3;


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

			vector<GLuint>  mGuardTextures;
			vector<GLuint> mPlayerTextures;

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
			std::vector<GuardObject*> mGuardObjects;

			InventoryBuffSystemClass* mInventoryBuffSystemClassPtr = nullptr;
			SuspicionSystemClass* mSuspicionSystemClassPtr = nullptr;

			std::map<PlayerInventory::item, Texture*> mItemTextureMap;
			// key variables
			int mActiveLevel;
			float mTimer;
			float mDtSinceLastFixedUpdate;
			GameStates mGameState;
			std::map<int, NetworkPlayer*>* serverPlayersPtr = nullptr;

			std::vector<PlayerInventoryObserver*> mPlayerInventoryObservers;
			std::vector<PlayerBuffsObserver*> mPlayerBuffsObservers;
		};
	}
}

