#pragma once
#include "TutorialGame.h"
#include "NetworkBase.h"

namespace NCL {
	namespace CSC8503 {
		class GameServer;
		class GameClient;
		class NetworkPlayer;

		enum NetworkRole {
			Server,
			Client
		};

		struct ClientInput {

			ClientInput() {

			}

			// struct that takes in user inputs to be sent from client to serve
			//
			// Author: Ewan Squire
			ClientInput(int lastID, char but1, char but2, char but3, char but4, char but5, char but6, char but7, char but8, Vector3 camPos) {
				this->lastID = lastID;
				buttonStates[0] = but1;
				buttonStates[1] = but2;
				buttonStates[2] = but3;
				buttonStates[3] = but4;
				buttonStates[4] = but5;
				buttonStates[5] = but6;
				buttonStates[6] = but7;
				buttonStates[7] = but8;
				cameraPosition = camPos;
			}

			int lastID;
			char buttonStates[8];
			Vector3 cameraPosition;
		};

		class NetworkedGame : public TutorialGame, public PacketReceiver {
		public:
			NetworkedGame();
			~NetworkedGame();

			void StartAsServer();
			void StartAsClient(char a, char b, char c, char d);

			void UpdateGame(float dt) override;

			void SpawnPlayer();

			void StartLevel();

			void ReceivePacket(int type, GamePacket* payload, int source) override;

			void OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b);

		protected:
			void UpdateAsServer(float dt);
			void UpdateAsClient(float dt);

			void BroadcastSnapshot(bool deltaFrame);
			void UpdateMinimumState();

			virtual void CreatePlayer1() = 0;
			virtual void CreatePlayer2() = 0;
			virtual void RotateCameraAroundPLayer(bool clockWise, float speed, float dt) = 0;

			virtual void SetItemsLeftToZero() = 0;

			std::map<int, int> stateIDs;

			GameServer* thisServer;
			GameClient* thisClient;
			float timeToNextPacket;
			int packetsToSnapshot;

			std::vector<NetworkObject*> networkObjects;

			std::map<int, GameObject*> serverPlayers;
			GameObject* localPlayer;

			// variables created by me to track network state and transfer data

			// server stores latest client ack of full state
			int latestClientFullState;
			// client stores ID of latest full state
			int latestServerFullState;
			int networkRole;

			ClientInput lastPlayer2Input;
		};
	}
}

