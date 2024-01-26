#pragma once

#include "NavigationGrid.h"
#include "PushdownMachine.h"
#include "PushdownState.h"
#include "NetworkedGame.h"
#include "NetworkObject.h"
#include "NetworkPlayer.h"
#include "StateMachine.h"
#include "State.h"
#include "StateTransition.h"

// class that generates levels and menus for the game
// All code related to this class is written by myself
//
// Author: Ewan Squire
namespace NCL {
	namespace CSC8503 {
		class LevelGenerator : public NetworkedGame
		{
		public:

			LevelGenerator();
			~LevelGenerator();

			void UpdateGame(float dt);

			void InitialiseAssets();

			// Menu functions
			void DisplayMainMenu();
			void DisplayPhysicsMenu();
			void DisplayPathfindingMenu();
			void DisplayNetworkingMenu();
			void DisplayWinScreen();
			void DisplayLoseScreen();

			// generate level functions
			void SetLevelToPhysics() { currentLevel = Physics; }
			void SetLevelToPathfinding() { currentLevel = Pathfinding; }
			void SetLevelToNetworking() { currentLevel = Networking; }

			void GeneratePhysicsLevel();
			void GeneratePathfindingLevel();
			void GenerateNetworkingLevel();

			// check if player hit ally or foe
			bool CheckPlayerHitEnemy();
			void CheckPlayerHitGoal();

			// get game variables
			int GetCurrentPoints() { return currentPoints; }
			int GetItemsLeft() { return itemsLeft; }
			void SetItemsLeftToZero() override { itemsLeft = 0; }
			int GetItemsGathered() { return itemsGathered; }
			float GetTimer() { return timer; }

			// get which player has won
			bool PlayerWin();

		protected:

			void UpdatePhysicsGame(float dt);
			void UpdatePathfindingGame(float dt);
			void UpdateNetworkingGame(float dt);

			// functions to create world
			void RotateCameraAroundPLayer(bool clockWise, float speed,float dt) override;

			void InitWorld();

			// character conroller functions

			void MoveCharacter(float dt, GameObject* characterControlled, float speed);

			void CharacterJump(float dt, GameObject* characterControlled);

			void CharacterCamera(float dt);

			void CharacterController(float dt, GameObject* characterController, float speed);

			Quaternion GetObjectRotation(Vector3 objectPos, Vector3 camPos);

			Quaternion Matrix3ToQuaternion(Matrix3 m);

			// generate game objects
			GameObject* GeneratePlayerObject(const Vector3& position, const std::string& objectName);

			GameObject* GenerateEnemyObject(const Vector3& position, const std::string& objectName);

			GameObject* GenerateGoal(Vector3 position, int level);

			// display function
			void DisplayPointsAndItemsLeft();

			void DisplayPlayer1Data();

			void DisplayPlayer2Data();

			// physics functions

			void GenerateWall(Vector2 bottonLeftPos, float componentSize, int wallXSize, int wallYSize, bool zAxis);

			void GeneratePhysicsLevelStaticWalls();
			
			// methods for pathfinding
			void GenerateLevelWalls(const std::string& filename);

			void SetUpAI();

			vector<Vector3> testNodes;
			void CreatePath();

			void MoveObjectOnPath(float speed);

			void ChaseObject(GameObject* chasing, GameObject* chased, float speed);

			// networking functions

			void CreatePlayer1() override;

			void CreatePlayer2() override;

			void UpdatePlayer2();

			void GenerateNetworkingLevelWalls();

			void GenerateNetworkingLevelGoals();

			// general use functions

			bool CheckVectorsEqual(const Vector3 vector1, const Vector3 vector2);

			Vector3 MakeForceUnfiform(Vector3 force, float magnitude);

			bool CheckForCollision(GameObject* a, GameObject* b);

			void EnemyChasePlayer();

			// game objects
			GameObject* playerObject;
			GameObject* enemyObject;

			// networking variables
			GameObject* player1Object;
			GameObject* player2Object;
			NetworkRole serverRole;

			// general variables
			int currentPoints;
			int itemsLeft;
			int itemsGathered;
			float timer;
			float playerSpeed;
			float enemySpeed;

			int player1Points;
			int player2Points;
			int player1Items;
			int player2Items;

			vector<GameObject*> physicsGoals;
			vector<GameObject*> pathfindingGoals;
			vector<GameObject*> networkingGoals;

			// state machine variables

			StateMachine* stateMachine;
			State* trackPlayer;
			State* chasePlayer;
			StateTransition* trackToChase;
			StateTransition* chaseToTrack;


			// variables for pathfinding
			int nodeSize;
			int gridWidth;
			int gridHeight;
			int currentNode;
		};
	}
}

