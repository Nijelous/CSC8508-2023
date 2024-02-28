#pragma once
#include "NetworkBase.h"
#include <stdint.h>
#include <thread>
#include <atomic>

namespace NCL::CSC8503{
	struct PlayerInputs;
}

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class GameClient : public NetworkBase {
		public:
			GameClient();
			~GameClient();

			int GetPeerID() const;

			bool Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum);

			void SendPacket(GamePacket&  payload);

			bool UpdateClient();

			void WriteAndSendClientInputPacket(int lastId, const PlayerInputs& playerInputs);

			void WriteAndSendClientUseItemPacket(int playerID, int objectID);
		protected:
			int mPeerId;
			
			_ENetPeer*	mNetPeer;
			float mTimerSinceLastPacket;
		};
	}
}

