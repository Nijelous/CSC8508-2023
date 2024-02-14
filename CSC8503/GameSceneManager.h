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
			LevelState,
			VictoryScreenState,
			DefeatScreenState
		};

		class GameSceneManager : public Scene{
		public:
			GameSceneManager(bool isNetworkGame = false);
			~GameSceneManager();

			virtual void UpdateGame(float dt);

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

			virtual void InitInGameMenuManager();

			void InitCamera();

			void InitIcons();

			void DisplayMainMenu();
			void DisplayVictory();
			void DisplayDefeat();
			
			GameStates mGameState;
			PushdownMachine* mPushdownMachine;

		private:
			
		};
	}
}
