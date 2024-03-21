#ifdef USEGL
#include "NetworkObject.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

SyncPlayerListPacket::SyncPlayerListPacket(std::vector<int>& serverPlayers) {
	type = BasicNetworkMessages::SyncPlayers;
	size = sizeof(SyncPlayerListPacket);

	for (int i = 0; i < 4; i++) {
		playerList[i] = serverPlayers[i];
	}
}

void SyncPlayerListPacket::SyncPlayerList(std::vector<int>& clientPlayerList) const
{
//TODO(erendgrmnc): Add config for max player number.
	
	for (int i = 0; i < 4; ++i) {
		clientPlayerList[i] = playerList[i];
	}
}

GameStartStatePacket::GameStartStatePacket(bool val, const std::string& seed) {
	type = BasicNetworkMessages::GameStartState;
	size = sizeof(GameStartStatePacket);

	isGameStarted = val;
	this->levelSeed = seed;
}

GameEndStatePacket::GameEndStatePacket(bool val, int winningPlayerId){
	type = BasicNetworkMessages::GameEndState;
	size = sizeof(GameEndStatePacket);

	this->isGameEnded = val;
	this->winningPlayerId = winningPlayerId;
}

ClientPlayerInputPacket::ClientPlayerInputPacket(int lastId, const PlayerInputs& playerInputs){
	type = BasicNetworkMessages::ClientPlayerInputState;
	size = sizeof(ClientPlayerInputPacket);

	this->playerInputs.isCrouching = playerInputs.isCrouching;
	this->playerInputs.isSprinting = playerInputs.isSprinting;
	this->playerInputs.isEquippedItemUsed = playerInputs.isEquippedItemUsed;
	this->playerInputs.isInteractButtonPressed = playerInputs.isInteractButtonPressed;
	this->playerInputs.isHoldingInteractButton = playerInputs.isHoldingInteractButton;

	this->playerInputs.leftHandItemId = playerInputs.leftHandItemId;
	this->playerInputs.rightHandItemId = playerInputs.rightHandItemId;
	
	this->playerInputs.movementButtons[0] = playerInputs.movementButtons[0];
	this->playerInputs.movementButtons[1] = playerInputs.movementButtons[1];
	this->playerInputs.movementButtons[2] = playerInputs.movementButtons[2];
	this->playerInputs.movementButtons[3] = playerInputs.movementButtons[3];

	this->playerInputs.fwdAxis.x = playerInputs.fwdAxis.x;
	this->playerInputs.fwdAxis.y = playerInputs.fwdAxis.y;
	this->playerInputs.fwdAxis.z = playerInputs.fwdAxis.z;

	this->playerInputs.rightAxis.x = playerInputs.rightAxis.x;
	this->playerInputs.rightAxis.y = playerInputs.rightAxis.y;
	this->playerInputs.rightAxis.z = playerInputs.rightAxis.z;
	this->playerInputs.cameraYaw = playerInputs.cameraYaw;

	this->playerInputs.rayFromPlayer = playerInputs.rayFromPlayer;
	
	this-> lastId = lastId;
	this->mouseXLook = mouseXLook;
}

ClientUseItemPacket::ClientUseItemPacket(int objectID, int playerID) {
	this->objectID = objectID;
	this->playerID = playerID;
}

ClientSyncBuffPacket::ClientSyncBuffPacket(int playerID, int buffID, bool toApply) {
	type = BasicNetworkMessages::ClientSyncBuffs;
	size = sizeof(ClientSyncBuffPacket);

	this->playerID = playerID;
	this->buffID = buffID;
	this->toApply = toApply;
}

ClientSyncLocalActiveSusCausePacket::ClientSyncLocalActiveSusCausePacket(int playerID, int activeLocalSusCauseID, bool toApply) {
	type = BasicNetworkMessages::ClientSyncLocalActiveCause;
	size = sizeof(ClientSyncLocalActiveSusCausePacket);

	this->playerID = playerID;
	this->activeLocalSusCauseID = activeLocalSusCauseID;
	this->toApply = toApply;
}

ClientSyncLocalSusChangePacket::ClientSyncLocalSusChangePacket(int playerID, int changedValue) {
	type = BasicNetworkMessages::ClientSyncLocalSusChange;
	size = sizeof(ClientSyncLocalSusChangePacket);

	this->playerID = playerID;
	this->changedValue = changedValue;
}

ClientSyncGlobalSusChangePacket::ClientSyncGlobalSusChangePacket(int changedValue) {
	type = BasicNetworkMessages::ClientSyncGlobalSusChange;
	size = sizeof(ClientSyncGlobalSusChangePacket);

	this->changedValue = changedValue;
}

ClientSyncLocationActiveSusCausePacket::ClientSyncLocationActiveSusCausePacket(int cantorPairedLocation, int activeLocationSusCauseID, bool toApply) {
	type = BasicNetworkMessages::ClientSyncLocationActiveCause;
	size = sizeof(ClientSyncLocationActiveSusCausePacket);

	this->cantorPairedLocation = cantorPairedLocation;
	this->activeLocationSusCauseID = activeLocationSusCauseID;
	this->toApply = toApply;
}

ClientSyncLocationSusChangePacket::ClientSyncLocationSusChangePacket(int cantorPairedLocation, int changedValue) {
	type = BasicNetworkMessages::ClientSyncLocationSusChange;
	size = sizeof(ClientSyncLocationSusChangePacket);

	this->cantorPairedLocation = cantorPairedLocation;
	this->changedValue = changedValue;
}

ClientSyncItemSlotUsagePacket::ClientSyncItemSlotUsagePacket(int playerID, int firstItemUsage, int secondItemUsage) {
	this->firstItemUsage = firstItemUsage;
	this->secondItemUsage = secondItemUsage;
	this->playerID = playerID;
}

ClientSyncItemSlotPacket::ClientSyncItemSlotPacket(int playerID, int slotId, int equippedItem, int usageCount) {
	type = BasicNetworkMessages::ClientSyncItemSlot;
	size = sizeof(ClientSyncItemSlotPacket);

	this->playerID = playerID;
	this->slotId = slotId;
	this->equippedItem = equippedItem;
	this->usageCount = usageCount;
}

SyncInteractablePacket::SyncInteractablePacket(int networkObjectId, bool isOpen, int interactableItemType) {
	type = BasicNetworkMessages::SyncInteractable;
	size = sizeof(SyncInteractablePacket);

	this->networkObjId = networkObjectId;
	this->isOpen = isOpen;
	this->interactableItemType = interactableItemType;
}

SyncObjectStatePacket::SyncObjectStatePacket(int networkObjId, int objectState) {
	type = SyncObjectState;
	size = sizeof(SyncObjectStatePacket);

	this->networkObjId = networkObjId;
	this->objectState = objectState;
}

ClientInitPacket::ClientInitPacket(const std::string& playerName) {
	type = ClientInit;
	size = sizeof(ClientInitPacket);

	this->playerName = playerName;
}

SyncPlayerIdNameMapPacket::SyncPlayerIdNameMapPacket(const std::map<int, string>& playerIdNameMap) {
	type = SyncPlayerIdNameMap;
	size = sizeof(SyncPlayerIdNameMapPacket);

	int counter = 0;
	for (std::pair<int, std::string> playerIdName : playerIdNameMap) {
		playerIds[counter] = playerIdName.first;
		playerNames[counter] = playerIdName.second;
		counter++;
	}

}

AnnouncementSyncPacket::AnnouncementSyncPacket(int annType, float time, int playerNo) {
	type = SyncAnnouncements;
	size = sizeof(AnnouncementSyncPacket);

	this->annType = annType;
	this->time = time;
	this->playerNo = playerNo;
}

GuardSpotSoundPacket::GuardSpotSoundPacket(const int playerId) {
	type = BasicNetworkMessages::GuardSpotSound;
	size = sizeof(GuardSpotSoundPacket);
	this->playerId = playerId;
}

NetworkObject::NetworkObject(GameObject& o, int id) : object(o) {
	deltaErrors = 0;
	fullErrors = 0;
	networkID = id;
}

NetworkObject::~NetworkObject() {
}

bool NetworkObject::ReadPacket(GamePacket& p) {
	// read packet depending on packet type
	// if neither it returns false

	if (p.type == Delta_State)
		return ReadDeltaPacket((DeltaPacket&)p);

	if (p.type == Full_State)
		return ReadFullPacket((FullPacket&)p);

	return false; //this isn't a packet we care about!
}

bool NetworkObject::WritePacket(GamePacket** p, bool deltaFrame, int stateID) {
	if (deltaFrame) {
		if (!WriteDeltaPacket(p, stateID)) {
			return WriteFullPacket(p);
		}
		return true;
	}
	return WriteFullPacket(p);
}
//Client objects recieve these packets
bool NetworkObject::ReadDeltaPacket(DeltaPacket &p) {
	// if the delta packets full state is not the same as the last examined full state we discard it
	if (p.fullID != lastFullState.stateID)
		return false;
	UpdateStateHistory(p.fullID);

	Vector3 fullPos = lastFullState.position;
	Quaternion fullOrientation = lastFullState.orientation;

	fullPos.x += p.pos[0];
	fullPos.y += p.pos[1];
	fullPos.z += p.pos[2];

	fullOrientation.x += ((float)p.orientation[0]) / 127.0f;
	fullOrientation.y += ((float)p.orientation[1]) / 127.0f;
	fullOrientation.z += ((float)p.orientation[2]) / 127.0f;
	fullOrientation.w += ((float)p.orientation[3]) / 127.0f;

	object.GetTransform().SetPosition(fullPos);
	object.GetTransform().SetOrientation(fullOrientation);
	return true;
}

bool NetworkObject::ReadFullPacket(FullPacket &p) {
	// if packet is old discard
	if (p.fullState.stateID < lastFullState.stateID)
		return false;

	lastFullState = p.fullState;

	object.GetTransform().SetPosition(lastFullState.position);
	object.GetTransform().SetOrientation(lastFullState.orientation);

	stateHistory.emplace_back(lastFullState);

	return true;
}

bool NetworkObject::WriteDeltaPacket(GamePacket**p, int stateID) {
	DeltaPacket* dp = new DeltaPacket();
	NetworkState state;

	// if we cant get network objects state we fail
	if (!GetNetworkState(stateID, state))
		return false;

	// tells packet what state it is a delta of
	dp->fullID = stateID;
	dp->objectID = networkID;

	Vector3 currentPos = object.GetTransform().GetPosition();
	Quaternion currentOrientation = object.GetTransform().GetOrientation();

	// find difference between current game states orientation + position and the selected states orientation + position
	currentPos -= state.position;
	currentOrientation -= state.orientation;

	dp->pos[0] = (char)currentPos.x;
	dp->pos[1] = (char)currentPos.y;
	dp->pos[2] = (char)currentPos.z;

	dp->orientation[0] = (char)(currentOrientation.x * 127.0f);
	dp->orientation[1] = (char)(currentOrientation.y * 127.0f);
	dp->orientation[2] = (char)(currentOrientation.z * 127.0f);
	dp->orientation[3] = (char)(currentOrientation.w * 127.0f);
	*p = dp;

	return true;
}

bool NetworkObject::WriteFullPacket(GamePacket**p) {
	FullPacket* fp = new FullPacket();


	fp->objectID = networkID;
	fp->fullState.position = object.GetTransform().GetPosition();
	fp->fullState.orientation = object.GetTransform().GetOrientation();
	fp->fullState.stateID = lastFullState.stateID++;
	stateHistory.emplace_back(fp->fullState);
	*p = fp;

	return true;
}

NetworkState& NetworkObject::GetLatestNetworkState() {
	return lastFullState;
}

bool NetworkObject::GetNetworkState(int stateID, NetworkState& state) {
	// get a state ID from state history if needed
	for (auto i = stateHistory.begin(); i < stateHistory.end(); i++) {
		if ((*i).stateID == stateID) {
			state = (*i);
			return true;
		}
	}
	return false;
}

void NetworkObject::UpdateStateHistory(int minID) {
	// once a client has accepted a delta packet or a network state has been
	// recieved then we can clear past state histories as they are not needed
	for (auto i = stateHistory.begin(); i < stateHistory.end();) {
		if ((*i).stateID < minID)
			i = stateHistory.erase(i);
		else
			i++;
	}
}
#endif