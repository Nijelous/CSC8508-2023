#ifdef USEGL
#include "DebugNetworkedGame.h"

#include <iostream>
#include <string>

#include "GameServer.h"
#include "GameClient.h"
#include "Helipad.h"
#include "Interactable.h"
#include "InteractableDoor.h"
#include "MultiplayerStates.h"
#include "NetworkObject.h"
#include "NetworkPlayer.h"
#include "PushdownMachine.h"
#include "RenderObject.h"
#include "LevelManager.h"
#include "../CSC8503/InventoryBuffSystem/InventoryBuffSystem.h"
#include "../CSC8503/InventoryBuffSystem/FlagGameObject.h"
#include "../CSC8503/SuspicionSystem/SuspicionSystem.h"
#include "Vent.h"
#include "Debug.h"

namespace {
	constexpr int MAX_PLAYER = 4;
	constexpr int DEMO_LEVEL_NUM = 0;
	constexpr int LEVEL_NUM = 1;
	constexpr int SERVER_PLAYER_PEER = 0;

	constexpr const char* PLAYER_PREFIX = "Player";

	//PLAYER MENU
	constexpr Vector4 LOCAL_PLAYER_COLOUR(0, 0, 1, 1);
	constexpr Vector4 DEFAULT_PLAYER_COLOUR(1, 1, 1, 1);

	constexpr float VERTICAL_MARGIN_BETWEEN_PLAYER_NAMES = 5.f;

}

DebugNetworkedGame::DebugNetworkedGame() {
	mThisServer = nullptr;
	mThisClient = nullptr;

	mClientSideLastFullID = 0;
	mServerSideLastFullID = 0;
	mGameState = GameSceneState::MainMenuState;

	NetworkBase::Initialise();
	mTimeToNextPacket = 0.0f;
	mPacketsToSnapshot = 0;
	mWinningPlayerId = -1;
	mLocalPlayerId = -1;

	bool isEmpty = mPacketToSendQueue.empty();
	mTimeToNextPacket = 0.0f;
	mPacketsToSnapshot = -1;
	InitInGameMenuManager();

	for (int i = 0; i < MAX_PLAYER; i++) {
		mPlayerList.push_back(-1);
	}
}

DebugNetworkedGame::~DebugNetworkedGame() {
}

bool DebugNetworkedGame::GetIsServer() const {
	return mIsServer;
}

bool DebugNetworkedGame::PlayerWonGame() {
	if (mIsGameFinished && mWinningPlayerId == mLocalPlayerId) {
		return true;
	}

	//TODO(erendgrmnc): lots of func calls, optimize it(ex: cache variables).
	std::tuple<bool, int> helipadCollisionResult = mLevelManager->GetHelipad()->GetCollidingWithPlayer();
	bool isAnyPlayerOnHelipad = std::get<0>(helipadCollisionResult);
	if (std::get<0>(helipadCollisionResult)) {
		int playerIDOnHelipad = std::get<1>(helipadCollisionResult);
		if (mLevelManager->GetInventoryBuffSystem()->GetPlayerInventoryPtr()->ItemInPlayerInventory(PlayerInventory::flag, playerIDOnHelipad)) {
			if (mIsServer) {
				SetIsGameFinished(true, playerIDOnHelipad);
			}
			if (mLocalPlayerId == playerIDOnHelipad) {
				return true;
			}
		}
	}
	return false;
}

bool DebugNetworkedGame::PlayerLostGame() {
	if (mIsGameFinished && mWinningPlayerId != mLocalPlayerId) {
		return true;
	}
	return false;
}

const bool DebugNetworkedGame::GetIsGameStarted() const {
	return mIsGameStarted;
}

bool DebugNetworkedGame::StartAsServer(const std::string& playerName) {
	mThisServer = new GameServer(NetworkBase::GetDefaultPort(), MAX_PLAYER);
	if (mThisServer) {

		mIsServer = true;

		mThisServer->RegisterPacketHandler(Received_State, this);
		mThisServer->RegisterPacketHandler(String_Message, this);
		mThisServer->RegisterPacketHandler(BasicNetworkMessages::ClientPlayerInputState, this);
		mThisServer->RegisterPacketHandler(BasicNetworkMessages::ClientInit, this);
		mThisServer->RegisterPacketHandler(BasicNetworkMessages::SyncAnnouncements, this);
		mThisServer->RegisterPacketHandler(BasicNetworkMessages::SyncInteractable, this);
		mThisServer->RegisterPacketHandler(BasicNetworkMessages::ClientSyncItemSlot, this);
		mThisServer->RegisterPacketHandler(BasicNetworkMessages::ClientSyncLocationSusChange, this);
		mThisServer->RegisterPacketHandler(BasicNetworkMessages::GuardSpotSound, this);

		AddToPlayerPeerNameMap(SERVER_PLAYER_PEER, playerName);

		std::thread senderThread(&DebugNetworkedGame::SendPacketsThread, this);
		senderThread.detach();
	}
	return mThisServer;
}

bool DebugNetworkedGame::StartAsClient(char a, char b, char c, char d, const std::string& playerName) {
	mThisClient = new GameClient();
	const bool isConnected = mThisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort(), playerName);

	if (isConnected) {
		mIsServer = false;
		mThisClient->RegisterPacketHandler(Delta_State, this);
		mThisClient->RegisterPacketHandler(Full_State, this);
		mThisClient->RegisterPacketHandler(Player_Connected, this);
		mThisClient->RegisterPacketHandler(Player_Disconnected, this);
		mThisClient->RegisterPacketHandler(String_Message, this);
		mThisClient->RegisterPacketHandler(GameStartState, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::SyncPlayers, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::GameEndState, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::ClientSyncItemSlotUsage, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::ClientSyncItemSlot, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::ClientSyncBuffs, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::ClientSyncLocalActiveCause, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::ClientSyncLocalSusChange, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::ClientSyncGlobalSusChange, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::ClientSyncLocationActiveCause, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::ClientSyncLocationSusChange, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::SyncInteractable, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::ClientSyncBuffs, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::SyncObjectState, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::SyncPlayerIdNameMap, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::SyncAnnouncements, this);
		mThisClient->RegisterPacketHandler(BasicNetworkMessages::GuardSpotSound, this);
	}

	return isConnected;

}

void DebugNetworkedGame::UpdateGame(float dt) {

	mTimeToNextPacket -= dt;
	if (mTimeToNextPacket < 0) {
		if (mThisServer) {
			UpdateAsServer(dt);
		}
		else if (mThisClient) {
			UpdateAsClient(dt);
		}
		mTimeToNextPacket += 1.0f / 60.0f; //20hz server/client update

		if (mThisServer && !mIsGameFinished) {
			SyncPlayerList();
		}
	}

	if (mPushdownMachine != nullptr) {
		mPushdownMachine->Update(dt);
	}

	if (mIsGameStarted && !mIsGameFinished) {

		ShowPlayerList();

		//TODO(erendgrmnc): rewrite this logic after end-game conditions are decided.

		mLevelManager->GetGameWorld()->GetMainCamera().UpdateCamera(dt);

		if (mThisServer) {
			Debug::Print("SERVER", Vector2(5, 10), Debug::MAGENTA);
		}
		else {
			Debug::Print("CLIENT", Vector2(5, 10), Debug::MAGENTA);
		}

		mLevelManager->Update(dt, mGameState == InitialisingLevelState, false);
	}
	else {
		mLevelManager->GetRenderer()->Render();
	}

	if (mThisServer) {
		mThisServer->UpdateServer();
	}
	if (mThisClient) {
		mThisClient->UpdateClient();
	}
}

void DebugNetworkedGame::SetIsGameStarted(bool isGameStarted, unsigned int seed) {
	if (mIsGameStarted == isGameStarted) {
		return;
	}
	this->mIsGameStarted = isGameStarted;

	int seedToUse = seed;
	if (isGameStarted) {
		mGameState = GameSceneState::InitialisingLevelState;
		if (mThisServer) {
			std::random_device rd;
			const unsigned int serverCreatedSeed = rd();

			const std::string seedString = std::to_string(serverCreatedSeed);

			SendStartGameStatusPacket(seedString);
			seedToUse = serverCreatedSeed;
		}

		std::mt19937 g(seedToUse);
		StartLevel(g);
	}
	else {
		if (mThisServer) {
			SendStartGameStatusPacket();
		}
	}
}

void DebugNetworkedGame::SetIsGameFinished(bool isGameFinished, int winningPlayerId) {
	mIsGameFinished = isGameFinished;
	mWinningPlayerId = winningPlayerId;
	if (mThisServer) {
		SendFinishGameStatusPacket();
	}
}

void DebugNetworkedGame::StartLevel(const std::mt19937& levelSeed) {
	InitWorld(levelSeed);
	Debug::Print("Game Started", Vector2(10, 5));

	for (auto& event : mOnGameStarts) {
		event();
	}
}

void DebugNetworkedGame::AddEventOnGameStarts(std::function<void()> event) {
	mOnGameStarts.push_back(event);
}

void DebugNetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
	
	switch (type) {
	case BasicNetworkMessages::GameStartState: {
		GameStartStatePacket* packet = (GameStartStatePacket*)payload;
		unsigned int seed = 0;

		seed = std::stoul(packet->levelSeed);
		SetIsGameStarted(packet->isGameStarted, seed);
		break;
	}
	case BasicNetworkMessages::Full_State: {
		FullPacket* packet = (FullPacket*)payload;
		HandleFullPacket(packet);
		break;
	}
	case BasicNetworkMessages::Delta_State: {
		DeltaPacket* deltaPacket = (DeltaPacket*)payload;
		HandleDeltaPacket(deltaPacket);
		break;
	}
	case BasicNetworkMessages::GameEndState: {
		GameEndStatePacket* packet = (GameEndStatePacket*)payload;
		SetIsGameFinished(packet->isGameEnded, packet->winningPlayerId);
		break;
	}
	case BasicNetworkMessages::SyncPlayers: {
		SyncPlayerListPacket* packet = (SyncPlayerListPacket*)payload;
		packet->SyncPlayerList(mPlayerList);
		break;
	}
	case  BasicNetworkMessages::ClientPlayerInputState: {
		ClientPlayerInputPacket* packet = (ClientPlayerInputPacket*)payload;
		HandleClientPlayerInputPacket(packet, source + 1);
		break;
	}
	case BasicNetworkMessages::ClientSyncItemSlot: {
		ClientSyncItemSlotPacket* packet = (ClientSyncItemSlotPacket*)(payload);
		HandlePlayerEquippedItemChange(packet);
		break;
	}
	case BasicNetworkMessages::SyncInteractable: {
		SyncInteractablePacket* packet = (SyncInteractablePacket*)(payload);
		HandleInteractablePacket(packet);
		break;
	}
	case BasicNetworkMessages::ClientSyncBuffs: {
		ClientSyncBuffPacket* packet = (ClientSyncBuffPacket*)(payload);
		HandlePlayerBuffChange(packet);
		break;
	}
	case BasicNetworkMessages::ClientSyncLocalActiveCause: {
		ClientSyncLocalActiveSusCausePacket* packet = (ClientSyncLocalActiveSusCausePacket*)(payload);
		HandleLocalActiveSusCauseChange(packet);
		break;
	}
	case BasicNetworkMessages::ClientSyncLocalSusChange: {
		ClientSyncLocalSusChangePacket* packet = (ClientSyncLocalSusChangePacket*)(payload);
		HandleLocalSusChange(packet);
		break;
	}
	case BasicNetworkMessages::ClientSyncGlobalSusChange: {
		ClientSyncGlobalSusChangePacket* packet = (ClientSyncGlobalSusChangePacket*)(payload);
		HandleGlobalSusChange(packet);
		break;
	}
	case BasicNetworkMessages::SyncObjectState: {
		SyncObjectStatePacket* packet = (SyncObjectStatePacket*)(payload);
		HandleObjectStatePacket(packet);
		break;
	}
	case BasicNetworkMessages::ClientInit: {
		ClientInitPacket* packet = (ClientInitPacket*)(payload);
		const int playerPeer = source + 1;
		HandleClientInitPacket(packet, playerPeer);
		break;
	}
	case BasicNetworkMessages::SyncPlayerIdNameMap: {
		const SyncPlayerIdNameMapPacket* packet = (SyncPlayerIdNameMapPacket*)(payload);
		HandleSyncPlayerIdNameMapPacket(packet);
		break;
	}
	case BasicNetworkMessages::SyncAnnouncements: {
		const AnnouncementSyncPacket* packet = (AnnouncementSyncPacket*)(payload);
		HandleAnnouncementSync(packet);
		break;
	}
	case BasicNetworkMessages::GuardSpotSound: {
		GuardSpotSoundPacket* packet = (GuardSpotSoundPacket*)(payload);
		HandleGuardSpotSound(packet);
		break;
	}
	default:
		std::cout << "Received unknown packet. Type: " << payload->type << std::endl;
		break;
	}
}

void DebugNetworkedGame::InitInGameMenuManager() {
	MultiplayerLobby* multiplayerLobby = new MultiplayerLobby(this);
	mPushdownMachine = new PushdownMachine(multiplayerLobby);
}

void DebugNetworkedGame::SendClientSyncItemSlotPacket(int playerNo, int invSlot, int inItem, int usageCount) const {
	PlayerInventory::item itemToEquip = (PlayerInventory::item)(inItem);
	NCL::CSC8503::ClientSyncItemSlotPacket packet(playerNo, invSlot, itemToEquip, usageCount);
	mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::SendClientSyncBuffPacket(int playerNo, int buffType, bool toApply) const {
	PlayerBuffs::buff buffToSync = (PlayerBuffs::buff)(buffType);
	NCL::CSC8503::ClientSyncBuffPacket packet(playerNo, buffToSync, toApply);
	mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::SendInteractablePacket(int networkObjectId, bool isOpen, int interactableItemType) const {
	SyncInteractablePacket packet(networkObjectId, isOpen, interactableItemType);
	mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::SendClientSyncLocalActiveSusCausePacket(int playerNo, int activeSusCause, bool toApply) const {
	LocalSuspicionMetre::activeLocalSusCause activeCause = (LocalSuspicionMetre::activeLocalSusCause)(activeSusCause);
	NCL::CSC8503::ClientSyncLocalActiveSusCausePacket packet(playerNo, activeCause, toApply);
	mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::SendClientSyncLocalSusChangePacket(int playerNo, int changedValue) const {
	NCL::CSC8503::ClientSyncLocalSusChangePacket packet(playerNo, changedValue);
	mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::SendClientSyncGlobalSusChangePacket(int changedValue) const {
	NCL::CSC8503::ClientSyncGlobalSusChangePacket packet(changedValue);
	mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::SendClientSyncLocationActiveSusCausePacket(int cantorPairedLocation, int activeSusCause, bool toApply) const {
	LocationBasedSuspicion::activeLocationSusCause activeCause = (LocationBasedSuspicion::activeLocationSusCause)(activeSusCause);
	NCL::CSC8503::ClientSyncLocationActiveSusCausePacket packet(cantorPairedLocation, activeCause, toApply);
	mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::SendClientSyncLocationSusChangePacket(int cantorPairedLocation, int changedValue) const {
	NCL::CSC8503::ClientSyncLocationSusChangePacket packet(cantorPairedLocation, changedValue);
	mThisServer->SendGlobalPacket(packet);
}

void NCL::CSC8503::DebugNetworkedGame::SendAnnouncementSyncPacket(int annType, float time, int playerNo){
	NCL::CSC8503::AnnouncementSyncPacket packet(annType,time, playerNo);
	mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::SendGuardSpotSoundPacket(int playerId) const {
	GuardSpotSoundPacket packet(playerId);
	mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::SendPacketsThread() {
	while (mThisServer) {
		std::lock_guard<std::mutex> lock(mPacketToSendQueueMutex);
		if (mPacketToSendQueue.size() > 1 && !mPacketToSendQueue.empty()) {
			
			GamePacket* packet = mPacketToSendQueue.front();
			if (packet) {
				mThisServer->SendGlobalPacket(*packet);
				mPacketToSendQueue.pop();
			}
		}
	}
}

GameClient* DebugNetworkedGame::GetClient() const {
	return mThisClient;
}

void DebugNetworkedGame::SendObjectStatePacket(int networkId, int state) const {
	NCL::CSC8503::SyncObjectStatePacket packet(networkId, state);
	mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::ClearNetworkGame() {
	if (mThisClient) {
		mThisClient->ClearPacketHandlers();
	}
	else {
		mThisServer->ClearPacketHandlers();
	}

	mServerPlayers.clear();

	mLevelManager->ClearLevel();

	mClientSideLastFullID = -1;
	mClientSideLastFullID = -1;
	mWinningPlayerId = -1;
	mNetworkObjectCache = 10;
}

GameServer* DebugNetworkedGame::GetServer() const {
	return mThisServer;
}

NetworkPlayer* DebugNetworkedGame::GetLocalPlayer() const {
	return static_cast<NetworkPlayer*>(mLocalPlayer);
}

void DebugNetworkedGame::UpdateAsServer(float dt) {
	mPacketsToSnapshot--;
	if (mPacketsToSnapshot < 0) {
		BroadcastSnapshot(false);
		mPacketsToSnapshot = 5;
	}
	else {
		BroadcastSnapshot(true);
	}
}

void DebugNetworkedGame::UpdateAsClient(float dt) {
	mThisClient->UpdateClient();
}

void DebugNetworkedGame::BroadcastSnapshot(bool deltaFrame) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	mLevelManager->GetGameWorld()->GetObjectIterators(first, last);

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
		if (o->WritePacket(&newPacket, deltaFrame, mServerSideLastFullID)) {
			if (newPacket != nullptr) {
				std::lock_guard<std::mutex> lock(mPacketToSendQueueMutex);
				mPacketToSendQueue.push(newPacket);
			}
		}
	}
}

void DebugNetworkedGame::UpdateMinimumState() {
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

int DebugNetworkedGame::GetPlayerPeerID(int peerId) {
	if (peerId == -2) {
		peerId = mThisClient->GetPeerID();
	}
	for (int i = 0; i < 4; ++i) {
		if (mPlayerList[i] == peerId) {
			return i;
		}
	}
	return -1;
}

const int DebugNetworkedGame::GetClientLastFullID() const {
	return mClientSideLastFullID;
}

void DebugNetworkedGame::SendStartGameStatusPacket(const std::string& seed) const {
	GameStartStatePacket state(mIsGameStarted, seed);
	mThisServer->SendGlobalPacket(state);
}

void DebugNetworkedGame::SendFinishGameStatusPacket() {
	GameEndStatePacket packet(mIsGameFinished, mWinningPlayerId);
	mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::InitWorld(const std::mt19937& levelSeed) {
	mLevelManager->GetGameWorld()->ClearAndErase();
	mLevelManager->GetPhysics()->Clear();

	mLevelManager->LoadLevel(LEVEL_NUM, levelSeed, 0, true);

	SpawnPlayers();

	mLevelManager->SetPlayersForGuards();

	mLevelManager->InitAnimationSystemObjects();
}

void DebugNetworkedGame::HandleClientPlayerInput(ClientPlayerInputPacket* playerMovementPacket, int playerPeerID) {
	//TODO(erendgrmc)
}

void DebugNetworkedGame::SpawnPlayers() {

	for (int i = 0; i < 4; i++) {
		if (mPlayerList[i] != -1) {

			const Vector3& pos = mLevelManager->GetPlayerStartPosition(i);
			auto* netPlayer = AddPlayerObject(pos, i);
			mServerPlayers.emplace(i, netPlayer);
			mLevelManager->GetInventoryBuffSystem()->GetPlayerInventoryPtr()->Attach(netPlayer);
			mLevelManager->GetInventoryBuffSystem()->GetPlayerBuffsPtr()->Attach(netPlayer);
		}
		else
		{
			mServerPlayers.emplace(i, nullptr);
		}
	}

	int playerPeerId = 0;

	if (!mThisServer) {
		playerPeerId = GetPlayerPeerID();
	}

	NetworkPlayer* localPlayer = mServerPlayers[playerPeerId];
	mLocalPlayer = localPlayer;

	mLocalPlayerId = mServerPlayers[playerPeerId]->GetPlayerID();
	localPlayer->SetIsLocalPlayer(true);
	mLevelManager->SetTempPlayer((PlayerObject*)mLocalPlayer);
	mLocalPlayer->ToggleIsRendered();
}

NetworkPlayer* DebugNetworkedGame::AddPlayerObject(const Vector3& position, int playerNum) {

	//Set Player Obj Name
	char buffer[256]; // Adjust the size according to your needs
	strcpy_s(buffer, sizeof(buffer), _strdup(PLAYER_PREFIX));
	strcat_s(buffer, sizeof(buffer), std::to_string(playerNum).c_str());

	auto* netPlayer = new NetworkPlayer(this, playerNum, buffer);
	mLevelManager->CreatePlayerObjectComponents(*netPlayer, position);

	auto* networkComponet = new NetworkObject(*netPlayer, playerNum);
	netPlayer->SetNetworkObject(networkComponet);
	mNetworkObjects.push_back(netPlayer->GetNetworkObject());
	mLevelManager->GetGameWorld()->AddGameObject(netPlayer);
	mLevelManager->AddUpdateableGameObject(*netPlayer);
	Vector4 colour;
	switch (playerNum)
	{
	case 0:
		netPlayer->GetRenderObject()->SetMatTextures(mLevelManager->GetMeshMaterial("Player_Red"));
		break;
	case 1:
		netPlayer->GetRenderObject()->SetMatTextures(mLevelManager->GetMeshMaterial("Player_Blue"));
		break;
	case 2:
		netPlayer->GetRenderObject()->SetMatTextures(mLevelManager->GetMeshMaterial("Player_Yellow"));
		break;
	case 3:
		netPlayer->GetRenderObject()->SetMatTextures(mLevelManager->GetMeshMaterial("Player_Green"));
		break;
	default:
		break;
	}
	
	return netPlayer;
}

void DebugNetworkedGame::HandleFullPacket(FullPacket* fullPacket) {
	for (int i = 0; i < mNetworkObjects.size(); i++) {
		if (mNetworkObjects[i]->GetnetworkID() == fullPacket->objectID) {
			mNetworkObjects[i]->ReadPacket(*fullPacket);
		}
	}
	mClientSideLastFullID = fullPacket->fullState.stateID;
}

void DebugNetworkedGame::HandleDeltaPacket(DeltaPacket* deltaPacket) {
	for (int i = 0; i < mNetworkObjects.size(); i++) {
		if (mNetworkObjects[i]->GetnetworkID() == deltaPacket->objectID) {
			mNetworkObjects[i]->ReadPacket(*deltaPacket);
		}
	}
}

void DebugNetworkedGame::HandleClientPlayerInputPacket(ClientPlayerInputPacket* clientPlayerInputPacket, int playerPeerId) {
	int playerIndex = GetPlayerPeerID(playerPeerId);
	auto* playerToHandle = mServerPlayers[playerIndex];

	playerToHandle->SetPlayerInput(clientPlayerInputPacket->playerInputs);
	mServerSideLastFullID = clientPlayerInputPacket->lastId;
	UpdateMinimumState();
}

void DebugNetworkedGame::HandleAddPlayerScorePacket(AddPlayerScorePacket* packet) {
}

void DebugNetworkedGame::SyncPlayerList() {
	int peerId;
	mPlayerList[0] = 0;
	for (int i = 0; i < 3; ++i) {
		if (mThisServer->GetPeer(i, peerId)) {
			mPlayerList[i + 1] = peerId;
		}
		else {
			mPlayerList[i + 1] = -1;
		}
	}

	SyncPlayerListPacket packet(mPlayerList);
	mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::SetItemsLeftToZero() {
}

void DebugNetworkedGame::HandlePlayerEquippedItemChange(ClientSyncItemSlotPacket* packet) const {
	const int localPlayerID = static_cast<NetworkPlayer*>(mLocalPlayer)->GetPlayerID();
	auto* inventorySystem = mLevelManager->GetInventoryBuffSystem()->GetPlayerInventoryPtr();
	const PlayerInventory::item equippedItem = static_cast<PlayerInventory::item>(packet->equippedItem);
	inventorySystem->ChangePlayerItem(packet->playerID, localPlayerID, packet->slotId, equippedItem, packet->usageCount);
}

void DebugNetworkedGame::HandleInteractablePacket(SyncInteractablePacket* packet) const {
	InteractableItems interactableItemType = static_cast<InteractableItems>(packet->interactableItemType);

	NetworkObject* interactedObj = nullptr;
	for (auto* networkObject : mNetworkObjects) {
		if (networkObject->GetnetworkID() == packet->networkObjId) {
			interactedObj = networkObject;
			break;
		}
	}

	switch (interactableItemType) {
	case InteractableItems::InteractableDoors: {
		InteractableDoor* doorObj = reinterpret_cast<InteractableDoor*>(interactedObj);
		doorObj->SyncDoor(packet->isOpen);
		break;
	}
	case InteractableItems::InteractableVents:{
		Vent* ventObj = reinterpret_cast<Vent*>(interactedObj);
		ventObj->SetIsOpen(packet->isOpen, false);
		break;
	}
	case InteractableItems::HeistItem:{
		LevelManager::GetLevelManager()->GetMainFlag()->Reset();
		break;
	}
	}
}
void DebugNetworkedGame::HandlePlayerBuffChange(ClientSyncBuffPacket* packet) const {
	const int localPlayerID = static_cast<NetworkPlayer*>(mLocalPlayer)->GetPlayerID();
	auto* buffSystem = mLevelManager->GetInventoryBuffSystem()->GetPlayerBuffsPtr();
	const PlayerBuffs::buff buffToSync = static_cast<PlayerBuffs::buff>(packet->buffID);
	buffSystem->SyncPlayerBuffs(packet->playerID, localPlayerID, buffToSync, packet->toApply);
}

void DebugNetworkedGame::HandleLocalActiveSusCauseChange(ClientSyncLocalActiveSusCausePacket* packet) const {
	const int localPlayerID = static_cast<NetworkPlayer*>(mLocalPlayer)->GetPlayerID();
	auto* localSusMetre = mLevelManager->GetSuspicionSystem()->GetLocalSuspicionMetre();
	const LocalSuspicionMetre::activeLocalSusCause activeCause = static_cast<LocalSuspicionMetre::activeLocalSusCause>(packet->activeLocalSusCauseID);
	localSusMetre->SyncActiveSusCauses(packet->playerID, localPlayerID, activeCause, packet->toApply);
}

void DebugNetworkedGame::HandleLocalSusChange(ClientSyncLocalSusChangePacket* packet) const {
	const int localPlayerID = static_cast<NetworkPlayer*>(mLocalPlayer)->GetPlayerID();
	auto* localSusMetre = mLevelManager->GetSuspicionSystem()->GetLocalSuspicionMetre();
	localSusMetre->SyncSusChange(packet->playerID, localPlayerID, packet->changedValue);
}

void DebugNetworkedGame::HandleGlobalSusChange(ClientSyncGlobalSusChangePacket* packet) const {
	auto* globalSusMetre = mLevelManager->GetSuspicionSystem()->GetGlobalSuspicionMetre();
	globalSusMetre->SyncSusChange(packet->changedValue);
}

void DebugNetworkedGame::HandleLocationActiveSusCauseChange(ClientSyncLocationActiveSusCausePacket* packet) const {
	const int localPlayerID = static_cast<NetworkPlayer*>(mLocalPlayer)->GetPlayerID();
	auto* locationSusMetre = mLevelManager->GetSuspicionSystem()->GetLocationBasedSuspicion();
	const LocationBasedSuspicion::activeLocationSusCause activeCause = static_cast<LocationBasedSuspicion::activeLocationSusCause>(packet->activeLocationSusCauseID);
	locationSusMetre->SyncActiveSusCauses(activeCause, packet->cantorPairedLocation, packet->toApply);
}

void DebugNetworkedGame::HandleLocationSusChange(ClientSyncLocationSusChangePacket* packet) const {
	auto* locationSusMetre = mLevelManager->GetSuspicionSystem()->GetLocationBasedSuspicion();
	locationSusMetre->SyncSusChange(packet->cantorPairedLocation, packet->changedValue);
}

void DebugNetworkedGame::HandleAnnouncementSync(const AnnouncementSyncPacket* packet) const{
	if (!mLocalPlayer)
		return;
	const PlayerObject::AnnouncementType annType = static_cast<PlayerObject::AnnouncementType>(packet->annType);
	GetLocalPlayer()->SyncAnnouncements(annType,packet->time,packet->playerNo);
}

void DebugNetworkedGame::HandleGuardSpotSound(GuardSpotSoundPacket* packet) const {
	if (packet->playerId == mLocalPlayerId) {
		mLevelManager->GetSoundManager()->PlaySpottedSound();
	}
}

void DebugNetworkedGame::AddToPlayerPeerNameMap(int playerId, const std::string& playerName) {
	mPlayerPeerNameMap.insert(std::pair<int, std::string>(playerId, playerName));
	if (mThisServer) {
		WriteAndSendSyncPlayerIdNameMapPacket();
	}
}

void DebugNetworkedGame::HandleClientInitPacket(const ClientInitPacket* packet, int playerID) {
	AddToPlayerPeerNameMap(playerID, packet->playerName);
}

void DebugNetworkedGame::WriteAndSendSyncPlayerIdNameMapPacket() const {
	SyncPlayerIdNameMapPacket packet(mPlayerPeerNameMap);
	mThisServer->SendGlobalPacket(packet);
}

void DebugNetworkedGame::HandleSyncPlayerIdNameMapPacket(const SyncPlayerIdNameMapPacket* packet) {
	mPlayerPeerNameMap.clear();
	for (int i = 0; i < 4; i++) {
		if (packet->playerIds[i] != -1) {
			std::pair<int, std::string> playerIdNamePair(packet->playerIds[i], packet->playerNames[i]);
			mPlayerPeerNameMap.insert(playerIdNamePair);
		}
	}
}

void DebugNetworkedGame::ShowPlayerList() const {
	if (Window::GetKeyboard()->KeyDown(KeyCodes::TAB)) {
		Vector2 position(15, 20);

		for (const std::pair<int, std::string>& playerIdNamePair : mPlayerPeerNameMap) {
			const Vector4& textColour = playerIdNamePair.first == mLocalPlayerId ? LOCAL_PLAYER_COLOUR : DEFAULT_PLAYER_COLOUR;

			std::stringstream ss;
			ss << playerIdNamePair.second << " ------- (" << playerIdNamePair.first << ")";
			Debug::Print(ss.str(), position, textColour);
			position.y += VERTICAL_MARGIN_BETWEEN_PLAYER_NAMES;
		}
	}
}

void DebugNetworkedGame::HandleObjectStatePacket(SyncObjectStatePacket* packet) const {
	NetworkObject* objectToChangeState = nullptr;
	for (NetworkObject* networkObject : mNetworkObjects) {
		if (networkObject->GetnetworkID() == packet->networkObjId) {
			objectToChangeState = networkObject;
			break;
		}
	}

	GameObject::GameObjectState state = static_cast<GameObject::GameObjectState>(packet->objectState);
	if (objectToChangeState != nullptr) {
		objectToChangeState->GetGameObject().SetObjectState(state);
	}
}
#endif
