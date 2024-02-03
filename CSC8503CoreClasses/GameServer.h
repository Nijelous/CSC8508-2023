#pragma once
#include "NetworkBase.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class GameServer : public NetworkBase {
		public:
			GameServer(int onPort, int maxClients);
			~GameServer();

			bool Initialise();
			void Shutdown();

			void SetGameWorld(GameWorld &g);
			void AddPeer(int peerNumber) const;

			bool SendGlobalPacket(int msgID);
			bool SendGlobalPacket(GamePacket& packet);
			bool SendVariableUpdatePacket(VariablePacket& packet);
			bool GetPeer(int peerNumber, int& peerId) const;

			virtual void UpdateServer();

		protected:
			int			port;
			int			clientMax;
			int			clientCount;
			int*        peers;
			GameWorld*	gameWorld;

			int incomingDataRate;
			int outgoingDataRate;
		};
	}
}
