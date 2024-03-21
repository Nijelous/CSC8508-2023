#ifdef USEGL

#include "GameClient.h"

#include "NetworkObject.h"
#include "../CSC8503/NetworkPlayer.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

GameClient::GameClient()	{
	netHandle = enet_host_create(nullptr, 1, 1, 0, 0);
	mTimerSinceLastPacket = 0.0f;
	mPeerId = -1;
	mIsConnected = false;
}

GameClient::~GameClient()	{
	enet_host_destroy(netHandle);
}

int GameClient::GetPeerID() const {
	return mPeerId;
}

bool GameClient::Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum, const std::string& playerName) {
	ENetAddress address;
	address.port = portNum;
	address.host = (d << 24) | (c << 16) | (b << 8) | (a);

	mNetPeer = enet_host_connect(netHandle, &address, 2, 0);
	mPlayerName = playerName;

	// returm false if net peer is null
	return mNetPeer != nullptr;
}

bool GameClient::UpdateClient() {
	// if there is no net handle we cannot handle packets
	if (netHandle == nullptr)
		return false;

	mTimerSinceLastPacket++;

	// handle incoming packets
	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0) {
		if (event.type == ENET_EVENT_TYPE_CONNECT) {
			//erendgrmnc: I remember +1 is needed because when counting server as a player, outgoing peer Id is not increasing.
			mPeerId = mNetPeer->outgoingPeerID + 1;
			mIsConnected = true;
			std::cout << "Connected to server!" << std::endl;

			//TODO(eren.degirmenci): send player init packet.
			SendClientInitPacket();
		}
		else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
			//std::cout << "Client Packet recieved..." << std::endl;
			GamePacket* packet = (GamePacket*)event.packet->data;
			ProcessPacket(packet);
			mTimerSinceLastPacket = 0.0f;
		}
		// once packet data is handled we can destroy packet and go to next
		enet_packet_destroy(event.packet);
	}
	// return false if client is no longer receiving packets
	if (mTimerSinceLastPacket > 20.0f) {
		return false;
	}
	return true;
}

void GameClient::WriteAndSendClientInputPacket(int lastId, const PlayerInputs& playerInputs){
	ClientPlayerInputPacket packet(lastId, playerInputs);
	this->SendPacket(packet);
}

void GameClient::SendPacket(GamePacket&  payload) {
	// defines packet to send and sends packet
	ENetPacket* dataPacket = enet_packet_create(&payload, payload.GetTotalSize(), 0);
	enet_peer_send(mNetPeer, 0, dataPacket);
}
void GameClient::Disconnect() {
	if (mNetPeer != nullptr) {
		// Disconnect from the server with a disconnect notification
		enet_peer_disconnect(mNetPeer, 0);

		// Allow up to 3 seconds for the disconnect to succeed and flush outgoing packets
		// You can adjust the timeout value as needed
		enet_host_flush(netHandle);

		// Wait until the disconnect process is complete or the timeout occurs
		ENetEvent event;
		if (enet_host_service(netHandle, &event, 3000) > 0 &&
			event.type == ENET_EVENT_TYPE_DISCONNECT) {
			// Disconnect successful
			std::cout << "Disconnected from the server." << std::endl;
		}
		else {
			// Disconnect timed out or encountered an error
			std::cerr << "Failed to disconnect from the server." << std::endl;
		}

		// Reset the peer to nullptr after disconnecting
		mNetPeer = nullptr;
	}
	mIsConnected = false;
}

bool GameClient::GetIsConnected() const {
	return mIsConnected;
}

void GameClient::SendClientInitPacket() {
	ClientInitPacket packet(mPlayerName);
	SendPacket(packet);
}

void GameClient::WriteAndSendAnnouncementSyncPacket(int annType, float time, int playerNo) {
	AnnouncementSyncPacket packet(annType, time, playerNo);
	this->SendPacket(packet);
}

void GameClient::WriteAndSendInteractablePacket(int networkObjectId, bool isOpen, int interactableItemType) {
	SyncInteractablePacket packet(networkObjectId, isOpen, interactableItemType);
	this->SendPacket(packet);
}
void GameClient::WriteAndSendInventoryPacket(int playerNo, int invSlot, int inItem, int usageCount){
	ClientSyncItemSlotPacket packet(playerNo, invSlot, inItem, usageCount);
	this->SendPacket(packet);
}
void GameClient::WriteAndSendSyncLocationSusChangePacket(int cantorPairedLocation, int changedValue) {
	ClientSyncLocationSusChangePacket packet(cantorPairedLocation, changedValue);
	this->SendPacket(packet);
}
#endif#
