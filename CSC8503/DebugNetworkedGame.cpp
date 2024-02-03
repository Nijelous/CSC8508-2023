#include "DebugNetworkedGame.h"

#include <iostream>
#include <string>

#include "GameServer.h"
#include "GameClient.h"
#include "NetworkObject.h"

namespace
{
    constexpr int MAX_PLAYER = 4;
}

DebugNetworkedGame::DebugNetworkedGame()
{
    thisServer = nullptr;
    thisClient = nullptr;

    NetworkBase::Initialise();
    timeToNextPacket = 0.0f;
    packetsToSnapshot = 0;

    for (int i = 0; i < MAX_PLAYER; i++){
        playerList.push_back(-1);
    }
}

DebugNetworkedGame::~DebugNetworkedGame() {
}

void DebugNetworkedGame::StartAsServer() {
    thisServer = new GameServer(NetworkBase::GetDefaultPort(), MAX_PLAYER);

    thisServer->RegisterPacketHandler(Received_State, this);
    thisServer->RegisterPacketHandler(String_Message, this);
}

void DebugNetworkedGame::StartAsClient(char a, char b, char c, char d)
{
    thisClient = new GameClient();
    int peer = thisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

    thisClient->RegisterPacketHandler(Delta_State, this);
    thisClient->RegisterPacketHandler(Full_State, this);
    thisClient->RegisterPacketHandler(Player_Connected, this);
    thisClient->RegisterPacketHandler(Player_Disconnected, this);
    thisClient->RegisterPacketHandler(String_Message, this);
    thisClient->RegisterPacketHandler(GameState, this);
    thisClient->RegisterPacketHandler(BasicNetworkMessages::SyncPlayers, this);
}

void DebugNetworkedGame::UpdateGame(float dt)
{
    timeToNextPacket -= dt;
    if (timeToNextPacket < 0) {
        if (thisServer) {
            UpdateAsServer(dt);
        }
        else if (thisClient) {
            UpdateAsClient(dt);
        }
        timeToNextPacket += 1.0f / 20.0f; //20hz server/client update

        if (thisServer)
        {
            SyncPlayerList();
        }
    }
    if (isGameStarted) {
        TutorialGame::UpdateGame(dt);
    }
    else
    {
        if (thisServer)
        {
            Debug::Print(" Waiting for player to join ...", Vector2(5, 95), Debug::RED);
            if (Window::GetKeyboard()->KeyPressed(KeyCodes::S))
            {
                SetIsGameStarted(true);
            }
        }
        else
        {
            Debug::Print(" Waiting for server to start ...", Vector2(5, 95), Debug::RED);
        }
        renderer->Render();
    }
    if (thisServer)
    {
        thisServer->UpdateServer();
    }
    if (thisClient)
    {
        thisClient->UpdateClient();
    }
}

void DebugNetworkedGame::SetIsGameStarted(bool isGameStarted) {
    this->isGameStarted = isGameStarted;
    if (thisServer) {
        SendGameStatusPacket();
    }
    if (isGameStarted){
        StartLevel();
    }
}

void DebugNetworkedGame::StartLevel() {
    Debug::Print("Game Started", Vector2(10, 5));
    InitWorld();
}

void DebugNetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
    switch (type)
    {
    case BasicNetworkMessages::String_Message: {
            int a = 0;
            break;
    }
    case BasicNetworkMessages::GameState: {
            GameStatePacket* packet = (GameStatePacket*)payload;
            SetIsGameStarted(packet->isGameStarted);
            break;
    }
    case BasicNetworkMessages::Full_State: {
            FullPacket* packet = (FullPacket*)payload;
            HandleFullPacket(packet); 
            break;
    }
    default:
        break;
    }
}

GameClient* DebugNetworkedGame::GetClient() const {
    return thisClient;
}

GameServer* DebugNetworkedGame::GetServer() const {
    return thisServer;
}

void DebugNetworkedGame::UpdateAsServer(float dt) {
    packetsToSnapshot--;
    if (packetsToSnapshot < 0) {
        BroadcastSnapshot(false);
        packetsToSnapshot = 5;
    }
    else {
        BroadcastSnapshot(true);
    }
}

void DebugNetworkedGame::UpdateAsClient(float dt) {
    thisClient->UpdateClient();

    int peer = GetPlayerPeerID();
    StringPacket newPacket("Client sends it regards :O ");
    if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
        thisClient->SendPacket(newPacket);
    }
}

void DebugNetworkedGame::BroadcastSnapshot(bool deltaFrame) {
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
        int playerState = o->GetLatestNetworkState().stateID;
        GamePacket* newPacket = nullptr;
        if (o->WritePacket(&newPacket, deltaFrame, playerState)) {
            thisServer->SendGlobalPacket(*newPacket);
            delete newPacket;
        }
    }
}

void DebugNetworkedGame::UpdateMinimumState() {
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

int DebugNetworkedGame::GetPlayerPeerID(int peerId) {
    if (peerId == -2) {
        peerId = thisClient->GetPeerID();
    }
    for (int i = 0; i < 4; ++i){
        if (playerList[i] == peerId)
        {
            return i;
        }
    }
    return -1;
}

void DebugNetworkedGame::SendGameStatusPacket() {
    GameStatePacket state(isGameStarted);
	thisServer->SendGlobalPacket(state);
}

void DebugNetworkedGame::InitWorld() {
    world->ClearAndErase();
    physics->Clear();

    InitDefaultFloor();
    
   //SpawnPlayers();
}

void DebugNetworkedGame::HandleClientPlayerInput(ClientPlayerInputPacket* playerMovementPacket, int playerPeerID) {
    //TODO(erendgrmc)
}

void DebugNetworkedGame::SpawnPlayers() {
    
}

NetworkPlayer* DebugNetworkedGame::AddPlayerObject(const Vector3& position, int playerNum)
{
    return nullptr;
}

void DebugNetworkedGame::HandleFullPacket(FullPacket* fullPacket) {
    for (int i = 0; i < networkObjects.size(); i++) {
        if (networkObjects[i]->GetnetworkID() == fullPacket->objectID) {
            networkObjects[i]->ReadPacket(*fullPacket);
        }
    }
}

void DebugNetworkedGame::HandleAddPlayerScorePacket(AddPlayerScorePacket* packet) {
    
}

void DebugNetworkedGame::SyncPlayerList() {
    int peerId;
    playerList[0] = 0;
    for (int i = 0; i < 3; ++i) {
        if (thisServer->GetPeer(i, peerId)) {
            playerList[i + 1] = peerId;
        }
        else{
            playerList[i + 1] = -1;
        }
    }

    SyncPlayerListPacket packet(playerList);
    thisServer->SendGlobalPacket(packet);
}
