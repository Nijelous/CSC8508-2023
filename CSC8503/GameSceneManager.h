#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once

#include "GameTechRenderer.h"

#include "PhysicsSystem.h"
#include "AnimationSystem.h"

namespace NCL {
	namespace CSC8503 {

		class PlayerObject;
		class GuardObject;
		class LevelManager;

		enum GameStates {
			MainMenuState,
			LevelState,
			VictoryScreenState,
			DefeatScreenState
		};

		class GameSceneManager {
		public:
			GameSceneManager();
			~GameSceneManager();

			virtual void UpdateGame(float dt);

			GameWorld* GetGameWorld() { return mWorld; }

			void SetMainMenu() { mGameState = MainMenuState; }
			void SetLevel() { mGameState = LevelState; }
			void SetVictory() { mGameState = VictoryScreenState; }
			void SetDefeat() { mGameState = DefeatScreenState; }

			// to be repalced by actual game logic
			bool PlayerWonGame() { return false; }
			bool PLayerLostGame() { return false; }

			void CreateLevel();

			void CreatePlayerObjectComponents(PlayerObject& playerObject, const Vector3& position) const;
			void CreateGuardObjectComponents(PlayerObject& playerObject, const Vector3& position) const;

		protected:
			void InitialiseAssets();

			void InitCamera();

			void DisplayMainMenu();
			void DisplayVictory();
			void DisplayDefeat();

			PlayerObject* AddPlayerToWorld(const Vector3 position, const std::string& playerName);

			GuardObject* AddGuardToWorld(const Vector3 position, const std::string& guardName);

			GameObject* AddFloorToWorld(const Vector3& position, const std::string& objectName);

			// world creation
			GameTechRenderer* mRenderer;
			PhysicsSystem* mPhysics;
			GameWorld* mWorld;
			LevelManager* mLevelManager;

			AnimationSystem* mAnimation;

			KeyboardMouseController mController;

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

			// animated meshes
			Mesh* mSoldierMesh;
			MeshAnimation* mSoldierAnimation;
			MeshMaterial* mSoldierMaterial;
			Shader* mSoldierShader;

			vector<GameObject*> mUpdatableObjects;

			GameStates mGameState;

			PlayerObject* mTempPlayer;

		private:
			
		};
	}
}