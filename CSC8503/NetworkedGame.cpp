#ifdef USEGL
#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"
#include "LevelManager.h"

#define COLLISION_MSG 30

struct MessagePacket : public GamePacket {
	short playerID;
	short messageID;

	MessagePacket() {
		type = Message;
		size = sizeof(short) * 2;
	}
};

NetworkedGame::NetworkedGame() : GameSceneManager(true) {
	mThisServer = nullptr;
	mThisClient = nullptr;

	NetworkBase::Initialise();
	mTimeToNextPacket  = 0.0f;
	mPacketsToSnapshot = 0;
}

NetworkedGame::~NetworkedGame()	{
	delete mThisServer;
	delete mThisClient;
}

void NetworkedGame::StartAsServer() {
	mThisServer = new GameServer(NetworkBase::GetDefaultPort(), 4);

	mThisServer->RegisterPacketHandler(Received_State, this);

	StartLevel();
}

void NetworkedGame::StartAsClient(char a, char b, char c, char d) {
	mThisClient = new GameClient();
	mThisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort(), "");

	mThisClient->RegisterPacketHandler(Delta_State, this);
	mThisClient->RegisterPacketHandler(Full_State, this);
	mThisClient->RegisterPacketHandler(Player_Connected, this);
	mThisClient->RegisterPacketHandler(Player_Disconnected, this);

	StartLevel();
}

void NetworkedGame::UpdateGame(float dt) {
	mTimeToNextPacket -= dt;
	if (mTimeToNextPacket < 0) {
		if (mThisServer) {
			UpdateAsServer(dt);
		}
		else if (mThisClient) {
			UpdateAsClient(dt);
		}
		mTimeToNextPacket += 1.0f / 20.0f; //20hz server/client update
	}

	if (!mThisServer && Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		std::cout << "Start As Server!" << std::endl;
	}
	if (!mThisClient && Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		std::cout << "Start As Client!" << std::endl;
	}

	GameSceneManager::UpdateGame(dt);
}

void NetworkedGame::UpdateAsServer(float dt) {
	mPacketsToSnapshot--;
	if (mPacketsToSnapshot < 0) {
	// full packet
	BroadcastSnapshot(false);
	mPacketsToSnapshot = 5;
	}
	else {
		// use delta packets
		//BroadcastSnapshot(true);
		
		// dont use delta packets
		BroadcastSnapshot(false);
	}

	mThisServer->UpdateServer();
}

// Tracks clients key presses and send sthem to server for it to compute
//
// Author: Ewan Squire
void NetworkedGame::UpdateAsClient(float dt) {
	ClientPacket newPacket;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
		//fire button pressed!
		newPacket.buttonstates[0] = ' ';
		// ID of last full state package recieved
	}
	newPacket.lastID = 0;

	mThisClient->SendPacket(newPacket);

	// if client no longer recieves packet then end game
	if (!mThisClient->UpdateClient()) {
		SetItemsLeftToZero();
	}
}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	mLevelManager->GetGameWorld()->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		// line added by me
		int playerState = 0;
		// line added by me
		GamePacket* newPacket = nullptr;
		if (o->WritePacket(&newPacket, deltaFrame, playerState)) {
			mThisServer->SendGlobalPacket(*newPacket);
			delete newPacket;
		}

	}
}

void NetworkedGame::UpdateMinimumState() {
	//Periodically remove old data from the server
	int minID = INT_MAX;
	int maxID = 0; //we could use this to see if a player is lagging behind?

	for (auto i : mStateIDs) {
		minID = std::min(minID, i.second);
		maxID = std::max(maxID, i.second);
	}
	//every client has acknowledged reaching at least state minID
	//so we can get rid of any old states!
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	mLevelManager->GetGameWorld()->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		o->UpdateStateHistory(minID); //clear out old states so they arent taking up memory...
	}
}

void NetworkedGame::SpawnPlayer() {

}

void NetworkedGame::StartLevel() {

}

// Server or client checks for incoming packets and unpacks them
//
// Author: Ewan Squire
void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
	
}

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b) {
	if (mThisServer) { //detected a collision between players!
		MessagePacket newPacket;
		newPacket.messageID = COLLISION_MSG;
		newPacket.playerID  = a->GetPlayerID();

		mThisClient->SendPacket(newPacket);

		newPacket.playerID = b->GetPlayerID();
		mThisClient->SendPacket(newPacket);
	}
}

void NetworkedGame::AddNetworkObjectToNetworkObjects(NetworkObject* networkObj) {
	mNetworkObjects.push_back(networkObj);
}
#endif