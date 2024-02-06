#include "DebugNetworkedGame.h"

#include <iostream>
#include <string>

#include "GameServer.h"
#include "GameClient.h"
#include "NetworkObject.h"

namespace{
    constexpr int MAX_PLAYER = 4;
}

DebugNetworkedGame::DebugNetworkedGame(){
    mThisServer = nullptr;
    mThisClient = nullptr;

    NetworkBase::Initialise();
    mTimeToNextPacket = 0.0f;
    mPacketsToSnapshot = 0;

    for (int i = 0; i < MAX_PLAYER; i++){
        mPlayerList.push_back(-1);
    }
}

DebugNetworkedGame::~DebugNetworkedGame(){
}

void DebugNetworkedGame::StartAsServer(){
    mThisServer = new GameServer(NetworkBase::GetDefaultPort(), MAX_PLAYER);

    mThisServer->RegisterPacketHandler(Received_State, this);
    mThisServer->RegisterPacketHandler(String_Message, this);
}

void DebugNetworkedGame::StartAsClient(char a, char b, char c, char d){
    mThisClient = new GameClient();
    int peer = mThisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

    mThisClient->RegisterPacketHandler(Delta_State, this);
    mThisClient->RegisterPacketHandler(Full_State, this);
    mThisClient->RegisterPacketHandler(Player_Connected, this);
    mThisClient->RegisterPacketHandler(Player_Disconnected, this);
    mThisClient->RegisterPacketHandler(String_Message, this);
    mThisClient->RegisterPacketHandler(GameStartState, this);
    mThisClient->RegisterPacketHandler(BasicNetworkMessages::SyncPlayers, this);
    mThisClient->RegisterPacketHandler(BasicNetworkMessages::GameEndState,this);
}

void DebugNetworkedGame::UpdateGame(float dt){
    mTimeToNextPacket -= dt;
    if (mTimeToNextPacket < 0){
        if (mThisServer){
            UpdateAsServer(dt);
        }
        else if (mThisClient){
            UpdateAsClient(dt);
        }
        mTimeToNextPacket += 1.0f / 20.0f; //20hz server/client update

        if (mThisServer){
            SyncPlayerList();
        }
    }
    if (isGameStarted){
        //TODO(erendgrmnc): rewrite this logic after end-game conditions are decided.
        if (mIsGameFinished){
            Debug::Print("Game Finished.", Vector2(5, 95), Debug::MAGENTA);
            renderer->Render();
            return;
        }
        
        //DEBUG END GAME
        if (mThisServer){
            if (Window::GetKeyboard()->KeyPressed(KeyCodes::S)){
                SetIsGameFinished(true);
            }
        }
        TutorialGame::UpdateGame(dt);
    }
    else{
        if (mThisServer){
            Debug::Print(" Waiting for player to join ...", Vector2(5, 95), Debug::RED);
            if (Window::GetKeyboard()->KeyPressed(KeyCodes::S)){
                SetIsGameStarted(true);
            }
        }
        else{
            Debug::Print(" Waiting for server to start ...", Vector2(5, 95), Debug::RED);
        }
        renderer->Render();
    }
    if (mThisServer){
        mThisServer->UpdateServer();
    }
    if (mThisClient){
        mThisClient->UpdateClient();
    }
}

void DebugNetworkedGame::SetIsGameStarted(bool isGameStarted){
    this->isGameStarted = isGameStarted;
    if (mThisServer){
        SendStartGameStatusPacket();
    }
    if (isGameStarted){
        StartLevel();
    }
}

void DebugNetworkedGame::SetIsGameFinished(bool isGameFinished){
    mIsGameFinished = isGameFinished;
    if (mThisServer){
        SendFinishGameStatusPacket();
    }
}

void DebugNetworkedGame::StartLevel(){
    Debug::Print("Game Started", Vector2(10, 5));
    
    InitWorld();

    for (auto& event : mOnGameStarts){
        event();
    }
}

void DebugNetworkedGame::AddEventOnGameStarts(std::function<void()> event){
    mOnGameStarts.push_back(event);
}

void DebugNetworkedGame::ReceivePacket(int type, GamePacket* payload, int source){
    switch (type){
    case BasicNetworkMessages::String_Message: {
        int a = 0;
        break;
    }
    case BasicNetworkMessages::GameStartState: {
        GameStartStatePacket* packet = (GameStartStatePacket*)payload;
        SetIsGameStarted(packet->isGameStarted);
        break;
    }
    case BasicNetworkMessages::Full_State: {
        FullPacket* packet = (FullPacket*)payload;
        HandleFullPacket(packet);
        break;
    }
    case BasicNetworkMessages::GameEndState:{
        GameEndStatePacket* packet = (GameEndStatePacket*)payload;
        SetIsGameFinished(packet->isGameEnded);
        break;
    }
    case BasicNetworkMessages::SyncPlayers:{
        break;
    }
    default:
        std::cout << "Received unknown packet. Type: " << payload->type  << std::endl;
        break;
    }
}

GameClient* DebugNetworkedGame::GetClient() const{
    return mThisClient;
}

GameServer* DebugNetworkedGame::GetServer() const{
    return mThisServer;
}

void DebugNetworkedGame::UpdateAsServer(float dt){
    mPacketsToSnapshot--;
    if (mPacketsToSnapshot < 0){
        BroadcastSnapshot(false);
        mPacketsToSnapshot = 5;
    }
    else{
        BroadcastSnapshot(true);
    }
}

void DebugNetworkedGame::UpdateAsClient(float dt){
    mThisClient->UpdateClient();

    int peer = GetPlayerPeerID();
    StringPacket newPacket("Client sends it regards :O ");
    if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)){
        mThisClient->SendPacket(newPacket);
    }
}

void DebugNetworkedGame::BroadcastSnapshot(bool deltaFrame){
    std::vector<GameObject*>::const_iterator first;
    std::vector<GameObject*>::const_iterator last;

    world->GetObjectIterators(first, last);

    for (auto i = first; i != last; ++i){
        NetworkObject* o = (*i)->GetNetworkObject();
        if (!o){
            continue;
        }
        //TODO - you'll need some way of determining
        //when a player has sent the server an acknowledgement
        //and store the lastID somewhere. A map between player
        //and an int could work, or it could be part of a 
        //NetworkPlayer struct. 
        int playerState = o->GetLatestNetworkState().stateID;
        GamePacket* newPacket = nullptr;
        if (o->WritePacket(&newPacket, deltaFrame, playerState)){
            mThisServer->SendGlobalPacket(*newPacket);
            delete newPacket;
        }
    }
}

void DebugNetworkedGame::UpdateMinimumState(){
    //Periodically remove old data from the server
    int minID = INT_MAX;
    int maxID = 0; //we could use this to see if a player is lagging behind?

    for (auto i : mStateIDs){
        minID = std::min(minID, i.second);
        maxID = std::max(maxID, i.second);
    }
    //every client has acknowledged reaching at least state minID
    //so we can get rid of any old states!
    std::vector<GameObject*>::const_iterator first;
    std::vector<GameObject*>::const_iterator last;
    world->GetObjectIterators(first, last);

    for (auto i = first; i != last; ++i){
        NetworkObject* o = (*i)->GetNetworkObject();
        if (!o){
            continue;
        }
        o->UpdateStateHistory(minID); //clear out old states so they arent taking up memory...
    }
}

int DebugNetworkedGame::GetPlayerPeerID(int peerId){
    if (peerId == -2){
        peerId = mThisClient->GetPeerID();
    }
    for (int i = 0; i < 4; ++i){
        if (mPlayerList[i] == peerId){
            return i;
        }
    }
    return -1;
}

void DebugNetworkedGame::SendStartGameStatusPacket(){
    GameStartStatePacket state(isGameStarted);
    mThisServer->SendGlobalPacket(state);
}

void DebugNetworkedGame::SendFinishGameStatusPacket(){
    GameEndStatePacket packet(mIsGameFinished);
    mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::InitWorld(){
    TutorialGame::InitWorld();
}

void DebugNetworkedGame::HandleClientPlayerInput(ClientPlayerInputPacket* playerMovementPacket, int playerPeerID){
    //TODO(erendgrmc)
}

void DebugNetworkedGame::SpawnPlayers(){
}

NetworkPlayer* DebugNetworkedGame::AddPlayerObject(const Vector3& position, int playerNum){
    return nullptr;
}

void DebugNetworkedGame::HandleFullPacket(FullPacket* fullPacket){
    for (int i = 0; i < mNetworkObjects.size(); i++){
        if (mNetworkObjects[i]->GetnetworkID() == fullPacket->objectID){
            mNetworkObjects[i]->ReadPacket(*fullPacket);
        }
    }
}

void DebugNetworkedGame::HandleAddPlayerScorePacket(AddPlayerScorePacket* packet){
}

void DebugNetworkedGame::SyncPlayerList(){
    int peerId;
    mPlayerList[0] = 0;
    for (int i = 0; i < 3; ++i){
        if (mThisServer->GetPeer(i, peerId)){
            mPlayerList[i + 1] = peerId;
        }
        else{
            mPlayerList[i + 1] = -1;
        }
    }

    SyncPlayerListPacket packet(mPlayerList);
    mThisServer->SendGlobalPacket(packet);
}
