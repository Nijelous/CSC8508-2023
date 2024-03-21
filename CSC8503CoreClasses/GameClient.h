#ifdef USEGL
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

			bool Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum, const std::string& playerName);

			void SendPacket(GamePacket&  payload);

			bool UpdateClient();

			void WriteAndSendClientInputPacket(int lastId, const PlayerInputs& playerInputs);

			void WriteAndSendClientUseItemPacket(int playerID, int objectID);

			void Disconnect();

			bool GetIsConnected() const;

			void WriteAndSendAnnouncementSyncPacket(int annType, float time, int playerNo);

			void WriteAndSendInteractablePacket(int networkObjectId, bool isOpen, int interactableItemType);

			void WriteAndSendInventoryPacket(int playerNo, int invSlot, int inItem, int usageCount);

			void WriteAndSendSyncLocationSusChangePacket(int cantorPairedLocation, int changedValue);
		protected:
			bool mIsConnected;

			int mPeerId;

			std::string mPlayerName;
			
			_ENetPeer*	mNetPeer;
			float mTimerSinceLastPacket;

			void SendClientInitPacket();
		};
	}
}

#endif