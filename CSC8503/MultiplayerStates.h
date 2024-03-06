#pragma once


#include "PushdownState.h"

namespace NCL {
	namespace CSC8503 {
		class DebugNetworkedGame;


		class MultiplayerLobby : public PushdownState {
		public:
			MultiplayerLobby(DebugNetworkedGame* currentGameState);

			PushdownResult OnUpdate(float dt, PushdownState** newState)  override;

			void OnAwake() override;

			DebugNetworkedGame* mGameSceneManager;
		};

		class InitialisingMultiplayerLevel : public PushdownState {
		public:
			InitialisingMultiplayerLevel(DebugNetworkedGame* currentGameState);

			PushdownResult OnUpdate(float dt, PushdownState** newState)  override;

			void OnAwake() override;

			DebugNetworkedGame* mGameSceneManager;
		};

		class PlayingMultiplayerLevel : public PushdownState {
		public:
			PlayingMultiplayerLevel(DebugNetworkedGame* currentGameState) {
				mGameSceneManager = currentGameState;
			}

			PushdownResult OnUpdate(float dt, PushdownState** newState)  override;

			void OnAwake() override;

			DebugNetworkedGame* mGameSceneManager;
		};

		class MultiplayerVictory : public PushdownState {
		public:
			MultiplayerVictory(DebugNetworkedGame* currentGameState) {
				mGameSceneManager = currentGameState;
			}

			PushdownResult OnUpdate(float dt, PushdownState** newState)  override;

			void OnAwake() override;

			DebugNetworkedGame* mGameSceneManager;
		};

		class MultiplayerDefeat : public PushdownState {
		public:
			MultiplayerDefeat(DebugNetworkedGame* currentGameState);

			PushdownResult OnUpdate(float dt, PushdownState** newState)  override;

			void OnAwake() override;

			DebugNetworkedGame* mGameSceneManager;
		};
	}
}