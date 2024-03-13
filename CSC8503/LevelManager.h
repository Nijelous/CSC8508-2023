#pragma once
#include "Level.h"
#ifdef USEGL
#include "GameTechRenderer.h"
#endif
#ifdef USEPROSPERO
#include "GameTechAGCRenderer.h"
#endif
#include "PhysicsSystem.h"
#include "AnimationSystem.h"
#include "RendererBase.h"
#include "InventoryBuffSystem/InventoryBuffSystem.h"
#include "InventoryBuffSystem/PlayerInventory.h"
#include "SuspicionSystem/SuspicionSystem.h"
#include "SoundManager.h"
#include <thread>

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
		class SuspicionSystem::GlobalSuspicionObserver;
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

			RendererBase* GetRenderer() { return mRenderer; }


			RecastBuilder* GetBuilder() { return mBuilder; }

			InventoryBuffSystemClass* GetInventoryBuffSystem();

			SuspicionSystemClass* GetSuspicionSystem();

			UISystem* GetUiSystem() { return mUi; };

#ifdef USEGL
			SoundManager* GetSoundManager() { return mSoundManager; };
			AnimationSystem* GetAnimationSystem() { return mAnimation; }
#endif

			virtual void UpdateInventoryObserver(InventoryEvent invEvent, int playerNo, int invSlot, bool isItemRemoved = false) override;

			const std::vector<Matrix4>& GetLevelFloorMatrices() { return mLevelFloorMatrices; }

			const std::vector<Matrix4>& GetLevelWallMatrices() { return mLevelWallMatrices; }

			const std::vector<Matrix4>& GetLevelCornerWallMatrices() { return mLevelCornerWallMatrices; }

			virtual void Update(float dt, bool isUpdatingObjects, bool isPaused);

			void FixedUpdate(float dt);

			void CreatePlayerObjectComponents(PlayerObject& playerObject, const Vector3& position);

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

			Helipad* GetHelipad(); 

			void LoadDoorInNavGrid(float* position, float* halfSize, PolyFlags flag);

			void SetGameState(GameStates state);

			void SetPlayersForGuards() const;

			PlayerObject* GetNearestPlayer(const Vector3& startPos) const;

			PrisonDoor* GetPrisonDoor() const;

			bool RoundHasStarted() { return mStartTimer <= 0; }
		protected:
			LevelManager();
			~LevelManager();

			static LevelManager* instance;

			virtual void InitialiseAssets();

			void InitialiseIcons();

			void LoadMap(const std::unordered_map<Transform, TileType>& tileMap, const Vector3& startPosition, int rotation = 0);

			void LoadLights(const std::vector<Light*>& lights, const Vector3& centre, int rotation = 0);

			void LoadGuards(int guardCount, bool isInMultiplayer);

			void LoadItems(const std::vector<Vector3>& itemPositions, const std::vector<Vector3>& roomItemPositions, const bool& isMultiplayer);

			void LoadVents(const std::vector<Vent*>& vents, const std::vector<int> ventConnections, bool isMultiplayerLevel = false);

			void LoadDoors(const std::vector<Door*>& doors, const Vector3& centre, bool isMultiplayerLevel = false, int rotation = 0);

			void LoadCCTVList(const std::vector<Transform>& transforms, const Vector3& startPosition, int rotation = 0);

			void LoadCCTVs();

			void LoadDoorsInNavGrid();

			void SendWallFloorInstancesToGPU();

			void AddNetworkObject(GameObject& objToAdd);

			GameObject* AddWallToWorld(const Transform& transform);
			GameObject* AddCornerWallToWorld(const Transform& transform);
			GameObject* AddFloorToWorld(const Transform& transform);
			CCTV* AddCCTVToWorld(const Transform& transform);
			Helipad* AddHelipadToWorld(const Vector3& position);
			Vent* AddVentToWorld(Vent* vent, bool isMultiplayerLevel = false);
			InteractableDoor* AddDoorToWorld(const Transform& transform, const Vector3& offset, bool isMultiplayerLevel = false);
			PrisonDoor* AddPrisonDoorToWorld(PrisonDoor* door);

			FlagGameObject* AddFlagToWorld(const Vector3& position, InventoryBuffSystemClass* inventoryBuffSystemClassPtr, SuspicionSystemClass* suspicionSystemClassPtr);

			PickupGameObject* AddPickupToWorld(const Vector3& position, InventoryBuffSystemClass* inventoryBuffSystemClassPtr, const bool& isMultiplayer);

			PointGameObject* AddPointObjectToWorld(const Vector3& position, int pointsWorth = 5, float initCooldown = 10);

			PlayerObject* AddPlayerToWorld(const Transform& transform, const std::string& playerName, PrisonDoor* mPrisonDoor);

			GuardObject* AddGuardToWorld(const vector<Vector3> nodes, const Vector3 prisonPosition, const std::string& guardName, bool isInMultiplayer);

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

#ifdef USEGL
			GameTechRenderer* mRenderer;
#endif

#ifdef USEPROSPERO
			GameTechAGCRenderer* mRenderer;
#endif

			GameWorld* mWorld;
			PhysicsSystem* mPhysics;
#ifdef USEGL // remove when converted to PS5 also
			AnimationSystem* mAnimation;
#endif

			SoundManager* mSoundManager;

			vector<GameObject*> mUpdatableObjects;

			std::unordered_map<std::string, Mesh*> mMeshes;
			std::unordered_map<std::string, Texture*> mTextures;
			std::unordered_map<std::string, Shader*> mShaders;
			std::unordered_map<std::string, MeshMaterial*> mMaterials;
			std::unordered_map<std::string, MeshAnimation*> mAnimations;

			std::vector<std::string> mShadersToLoad;

			UISystem* mUi;

			FlagGameObject* mMainFlag;

#ifdef USEGL
			vector<GLuint>  mGuardTextures;
			vector<GLuint> mPlayerTextures;
#endif
#ifdef USEPROSPERO
			// PSSL textures
#endif

			//animation guard
			std::map<std::string, MeshAnimation*> mPreAnimationList;

			// game objects
			Helipad* mHelipad;
			PlayerObject* mTempPlayer;
			PrisonDoor* mPrisonDoor;
			std::vector<GuardObject*> mGuardObjects;
			std::vector<Transform> mCCTVTransformList;

			InventoryBuffSystemClass* mInventoryBuffSystemClassPtr = nullptr;
			SuspicionSystemClass* mSuspicionSystemClassPtr = nullptr;

			std::map<PlayerInventory::item, Texture*> mItemTextureMap;
			// key variables
			int mActiveLevel;
			int mNetworkIdBuffer;
			float mStartTimer;
			float mTimer;
			float mDtSinceLastFixedUpdate;
			GameStates mGameState;
			std::map<int, NetworkPlayer*>* serverPlayersPtr = nullptr;

			std::thread mNavMeshThread;

			bool mIsLevelInitialised;

			std::vector<PlayerInventoryObserver*> mPlayerInventoryObservers;
			std::vector<PlayerBuffsObserver*> mPlayerBuffsObservers;
			std::vector<GlobalSuspicionObserver*> mGlobalSuspicionObserver;
		};
	}
}

