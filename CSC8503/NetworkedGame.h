#ifdef USEGL
#pragma once
#include "GameSceneManager.h"
#include "NetworkBase.h"
#include "Vector3.h"

namespace NCL {
	namespace CSC8503 {
		class GameServer;
		class GameClient;
		class GameObject;
		class NetworkPlayer;
		class NetworkObject;

		struct ClientInput {

			// struct that takes in user inputs to be sent from client to serve
			//
			// Author: Ewan Squire
			ClientInput(int lastID, char but1, char but2, char but3, char but4, char but5, char but6, char but7, char but8, Maths::Vector3 camPos) {
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
			Maths::Vector3 cameraPosition;
		};

		class NetworkedGame : public GameSceneManager, public PacketReceiver {
		public:
			NetworkedGame();
			virtual ~NetworkedGame();

			void StartAsServer();
			void StartAsClient(char a, char b, char c, char d);

			void UpdateGame(float dt) override;

			void SpawnPlayer();

			void StartLevel();

			void ReceivePacket(int type, GamePacket* payload, int source) override;

			void OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b);

			void AddNetworkObjectToNetworkObjects(NetworkObject* networkObj);

			std::map<int, NetworkPlayer*>* GetServerPlayersPtr() { return &mServerPlayers;  };

		protected:
			void UpdateAsServer(float dt);
			void UpdateAsClient(float dt);

			void BroadcastSnapshot(bool deltaFrame);
			void UpdateMinimumState();

			virtual void SetItemsLeftToZero() = 0;

            std::map<int, int> mStateIDs;

            GameServer* mThisServer = nullptr;
            GameClient* mThisClient = nullptr;
            float mTimeToNextPacket;
            int mPacketsToSnapshot;

            std::vector<NetworkObject*> mNetworkObjects;

            std::vector<int> mPlayerList;
            std::map<int, NetworkPlayer*> mServerPlayers;
            GameObject* mLocalPlayer;
		};
	}
}

#endif