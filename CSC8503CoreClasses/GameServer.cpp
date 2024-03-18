#ifdef USEGL
#include "GameServer.h"
#include "GameWorld.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

GameServer::GameServer(int onPort, int maxClients)	{
	mPort		= onPort;
	mClientMax	= maxClients;
	mClientCount = 0;
	netHandle	= nullptr;
	mPeers = new int[mClientMax];
	for (int i = 0; i < mClientMax; ++i){
		mPeers[i] = -1;
	}
	Initialise();
}

GameServer::~GameServer()	{
	Shutdown();
}

void GameServer::Shutdown() {
	SendGlobalPacket(BasicNetworkMessages::Shutdown);
	enet_host_destroy(netHandle);
	netHandle = nullptr;
}

bool GameServer::Initialise() {
	// create game server
	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = mPort;

	netHandle = enet_host_create(&address, mClientMax, 1, 0, 0);

	// if server is not set up then diplay error message and return false
	if (!netHandle) {
		std::cout << __FUNCTION__ << "failed to create network handle!" << std::endl;
		return false;
	}

	return true;
}

bool GameServer::SendGlobalPacket(int msgID) {
	GamePacket packet;
	packet.type = msgID;
	return SendGlobalPacket(packet);
}

bool GameServer::SendGlobalPacket(GamePacket& packet) {
	// define and send packet
	ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);
	enet_host_broadcast(netHandle, 0, dataPacket);
	return true;
}

bool GameServer::SendVariableUpdatePacket(VariablePacket& packet) {
	ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);
	enet_host_broadcast(netHandle, 0, dataPacket);
	return true;
}

bool GameServer::GetPeer(int peerNumber, int& peerId) const
{
	if (peerNumber >= mClientMax)
		return false;
	if (mPeers[peerNumber] == -1) {
		return false;
	}
	peerId = mPeers[peerNumber];
	return true;
}

void GameServer::UpdateServer() {
	if (!netHandle) { return; }

	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0) {
		int type = event.type;
		ENetPeer* p = event.peer;
		int peer = p->incomingPeerID;

		if (type == ENetEventType::ENET_EVENT_TYPE_CONNECT) {
			std::cout << "Server: New client has connected" << std::endl;
			AddPeer(peer + 1);
		}
		else if (type == ENetEventType::ENET_EVENT_TYPE_DISCONNECT) {
			std::cout << "Server: Client has disconnected" << std::endl;
			for (int i = 0; i < 3; ++i){
				if (mPeers[i] == peer+1) {
					mPeers[i] = -1;
				}
			}

		}
		else if (type == ENetEventType::ENET_EVENT_TYPE_RECEIVE) {
			//std::cout << "Server: Has recieved packet" << std::endl;
			GamePacket* packet = (GamePacket*)event.packet->data;
			ProcessPacket(packet, peer);
		}
		enet_packet_destroy(event.packet);
	}
}

void GameServer::SetGameWorld(GameWorld &g) {
	mGameWorld = &g;
}

void GameServer::AddPeer(int peerNumber) const
{
	int emptyIndex = mClientMax;
	for (int i = 0; i < mClientMax; i++) {
		if (mPeers[i] == peerNumber){
			return;
		}
		if (mPeers[i] == -1) {
			emptyIndex = std::min(i, emptyIndex);
		}
	}
	if (emptyIndex < mClientMax){
		mPeers[emptyIndex] = peerNumber;
	}
}
#endif