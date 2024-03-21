#pragma once

#include "Scene.h"

namespace NCL::CSC8503{
	class PushdownMachine;
}

namespace NCL {
	namespace CSC8503 {

		class PlayerObject;
		class GuardObject;

		enum GameSceneState {
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
			virtual bool PlayerWonGame();
			virtual bool PlayerLostGame();

			void CreateLevel();

		protected:
			virtual void InitWorld() {}

			virtual void InitInGameMenuManager();

			void InitCamera();

			void DisplayMainMenu();
			void DisplayVictory();
			void DisplayDefeat();
			void DisplayPauseScreen();
			
			GameSceneState mGameState;
			PushdownMachine* mPushdownMachine;

		private:
			static GameSceneManager* instance;

			int mPlayersFinalPoints;
		};
	}
}
