#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once

#include "LevelManager.h"

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

			LevelManager* GetLevelManager() { return mLevelManager; }

			void SetMainMenu() { mGameState = MainMenuState; }
			void SetLevel() { mGameState = LevelState; }
			void SetVictory() { mGameState = VictoryScreenState; }
			void SetDefeat() { mGameState = DefeatScreenState; }

			// to be repalced by actual game logic
			bool PlayerWonGame();
			bool PLayerLostGame() { return false; }

			void CreateLevel();

		protected:
			virtual void InitWorld() {}

			void InitCamera();

			void DisplayMainMenu();
			void DisplayVictory();
			void DisplayDefeat();

			// world creation
			LevelManager* mLevelManager;

			KeyboardMouseController mController;

			GameStates mGameState;

		private:
			
		};
	}
}