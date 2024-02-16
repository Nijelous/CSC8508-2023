#pragma once

#include "LevelManager.h"
#include "Scene.h"

namespace NCL::CSC8503{
	class PushdownMachine;
}

namespace NCL {
	namespace CSC8503 {

		class PlayerObject;
		class GuardObject;
		class LevelManager;

		enum GameStates {
			MainMenuState,
			InitialisingLevelState,
			PlayingLevelState,
			VictoryScreenState,
			DefeatScreenState,
			PauseScreenState
		};

		class GameSceneManager : public Scene {
		public:
			GameSceneManager(bool isNetworkGame = false);
			~GameSceneManager();

			GameSceneManager GetGameSceneManager();

			virtual void UpdateGame(float dt);

			void SetMainMenu() { mGameState = MainMenuState; }
			void SetInitLevel() { mGameState = InitialisingLevelState; }
			void SetLevel() { mGameState = PlayingLevelState; }
			void SetVictory() { mGameState = VictoryScreenState; }
			void SetDefeat() { mGameState = DefeatScreenState; }
			void SetPause() { mGameState = PauseScreenState; }

			// to be repalced by actual game logic
			bool PlayerWonGame();
			bool PLayerLostGame();

			void CreateLevel();

		protected:
			virtual void InitWorld() {}

			virtual void InitInGameMenuManager();

			void InitCamera();

			void InitIcons();

			void DisplayMainMenu();
			void DisplayVictory();
			void DisplayDefeat();
			
			GameStates mGameState;
			PushdownMachine* mPushdownMachine;

		private:
			static GameSceneManager* instance;

			int mPlayersFinalPoints;
		};
	}
}
