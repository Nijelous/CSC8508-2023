#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"

#define COLLISION_MSG 30

struct MessagePacket : public GamePacket {
	short playerID;
	short messageID;

	MessagePacket() {
		type = Message;
		size = sizeof(short) * 2;
	}
};

NetworkedGame::NetworkedGame() {
	thisServer = nullptr;
	thisClient = nullptr;

	NetworkBase::Initialise();
	timeToNextPacket  = 0.0f;
	packetsToSnapshot = 0;
}

NetworkedGame::~NetworkedGame()	{
	delete thisServer;
	delete thisClient;
}

void NetworkedGame::StartAsServer() {
	thisServer = new GameServer(NetworkBase::GetDefaultPort(), 4);

	thisServer->RegisterPacketHandler(Received_State, this);

	StartLevel();

	networkRole = Server;
}

void NetworkedGame::StartAsClient(char a, char b, char c, char d) {
	thisClient = new GameClient();
	thisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

	thisClient->RegisterPacketHandler(Delta_State, this);
	thisClient->RegisterPacketHandler(Full_State, this);
	thisClient->RegisterPacketHandler(Player_Connected, this);
	thisClient->RegisterPacketHandler(Player_Disconnected, this);

	StartLevel();

	networkRole = Client;
}

void NetworkedGame::UpdateGame(float dt) {
	timeToNextPacket -= dt;
	if (timeToNextPacket < 0) {
		if (thisServer) {
			UpdateAsServer(dt);
		}
		else if (thisClient) {
			UpdateAsClient(dt);
		}
		timeToNextPacket += 1.0f / 20.0f; //20hz server/client update
	}
	if (currentLevel == Networking) {
		if (!thisServer && Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
			CreatePlayer1();
		}
		if (!thisClient && Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
			CreatePlayer2();
		}
	}

	TutorialGame::UpdateGame(dt);
}

void NetworkedGame::UpdateAsServer(float dt) {
	packetsToSnapshot--;
	if (packetsToSnapshot < 0) {
	// full packet
	BroadcastSnapshot(false);
	packetsToSnapshot = 5;
	}
	else {
		// delta packet
		// use delta packets
		//BroadcastSnapshot(true);
		// dont use delta packets
		BroadcastSnapshot(false);
	}

	thisServer->UpdateServer();
}
void NetworkedGame::UpdateAsClient(float dt) {
	ClientPacket newPacket;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
		//fire button pressed!
		newPacket.buttonstates[0] = ' ';
		// ID of last full state package recieved
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) {
		newPacket.buttonstates[1] = 'W';
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
		newPacket.buttonstates[2] = 'A';
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) {
		newPacket.buttonstates[3] = 'S';
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
		newPacket.buttonstates[4] = 'D';
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::SPACE)) {
		newPacket.buttonstates[5] = ' ';
	}

	if (!inSelectionMode) {
		if (controller.GetAxis(3) > 0.0f)
			RotateCameraAroundPLayer(true, controller.GetAxis(3) * 1, dt);
		if (controller.GetAxis(3) < 0.0f)
			RotateCameraAroundPLayer(false, controller.GetAxis(3) * -1, dt);
	}
	newPacket.lastID = latestServerFullState;
	newPacket.cameraPosition = world->GetMainCamera().GetPosition();

	thisClient->SendPacket(newPacket);

	// if client no longer recieves packet then end game
	if (!thisClient->UpdateClient()) {
		SetItemsLeftToZero();
	}
}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		//TODO - you'll need some way of determining
		//when a player has sent the server an acknowledgement
		//and store the lastID somewhere. A map between player
		//and an int could work, or it could be part of a 
		//NetworkPlayer struct.
		// 
		// if not a delta packet (therefore full packet) add to full state
		int playerState = latestClientFullState;
		GamePacket* newPacket = nullptr;
		if (o->WritePacket(&newPacket, deltaFrame, playerState)) {
			thisServer->SendGlobalPacket(*newPacket);
			delete newPacket;
		}

	}
}

void NetworkedGame::UpdateMinimumState() {
	//Periodically remove old data from the server
	int minID = INT_MAX;
	int maxID = 0; //we could use this to see if a player is lagging behind?

	for (auto i : stateIDs) {
		minID = std::min(minID, i.second);
		maxID = std::max(maxID, i.second);
	}
	//every client has acknowledged reaching at least state minID
	//so we can get rid of any old states!
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	world->GetObjectIterators(first, last);

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

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
	if (type == Full_State) {
		FullPacket* realPacket = (FullPacket*)payload;
		// if client 
		latestServerFullState = realPacket->fullState.stateID;

		std::vector<GameObject*>::const_iterator first;
		std::vector<GameObject*>::const_iterator last;

		world->GetObjectIterators(first, last);
		for (auto i = first; i != last; ++i) {
			NetworkObject* o = (*i)->GetNetworkObject();
			if (!o) {
				continue;
			}
			if (o->GetnetworkID() == realPacket->objectID) {
				o->ReadPacket(*payload);
			}
		}
	}
	if (type == Delta_State) {
		DeltaPacket* realPacket = (DeltaPacket*)payload;

		std::vector<GameObject*>::const_iterator first;
		std::vector<GameObject*>::const_iterator last;

		world->GetObjectIterators(first, last);
		for (auto i = first; i != last; ++i) {
			NetworkObject* o = (*i)->GetNetworkObject();
			if (!o) {
				continue;
			}
			if (o->GetnetworkID() == realPacket->objectID) {
				o->ReadPacket(*payload);
			}
		}
	}
	if (type == Received_State) {
		// client -> Server packet
		ClientPacket* realPacket = (ClientPacket*)payload;
		lastPlayer2Input = ClientInput(	realPacket->lastID, 
										realPacket->buttonstates[0], 
										realPacket->buttonstates[1],
										realPacket->buttonstates[2],
										realPacket->buttonstates[3],
										realPacket->buttonstates[4],
										realPacket->buttonstates[5],
										realPacket->buttonstates[6],
										realPacket->buttonstates[7],
										realPacket->cameraPosition);
		latestClientFullState = realPacket->lastID;
	}
}

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b) {
	if (thisServer) { //detected a collision between players!
		MessagePacket newPacket;
		newPacket.messageID = COLLISION_MSG;
		newPacket.playerID  = a->GetPlayerNum();

		thisClient->SendPacket(newPacket);

		newPacket.playerID = b->GetPlayerNum();
		thisClient->SendPacket(newPacket);
	}
}