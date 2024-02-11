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

GameStartStatePacket::GameStartStatePacket(bool val) {
	type = BasicNetworkMessages::GameStartState;
	size = sizeof(GameStartStatePacket);

	isGameStarted = val;
}

GameEndStatePacket::GameEndStatePacket(bool val){
	type = BasicNetworkMessages::GameEndState;
	size = sizeof(GameEndStatePacket);

	isGameEnded = val;
}

ClientPlayerInputPacket::ClientPlayerInputPacket(int lastId, const PlayerInputs& playerInputs){
	type = BasicNetworkMessages::ClientPlayerInputState;
	size = sizeof(ClientPlayerInputPacket);

	this->playerInputs.isCrouching = playerInputs.isCrouching;
	this->playerInputs.isSprinting = playerInputs.isSprinting;
	this->playerInputs.isLeftHandUsed = playerInputs.isLeftHandUsed;
	this->playerInputs.isRightHandUsed = playerInputs.isRightHandUsed;

	this->playerInputs.leftHandItemId = playerInputs.leftHandItemId;
	this->playerInputs.rightHandItemId = playerInputs.rightHandItemId;
	
	this->playerInputs.movementButtons[0] = playerInputs.movementButtons[0];
	this->playerInputs.movementButtons[1] = playerInputs.movementButtons[1];
	this->playerInputs.movementButtons[2] = playerInputs.movementButtons[3];
	this->playerInputs.movementButtons[3] = playerInputs.movementButtons[3];

	this->playerInputs.fwdAxis.x = playerInputs.fwdAxis.x;
	this->playerInputs.fwdAxis.y = playerInputs.fwdAxis.y;
	this->playerInputs.fwdAxis.z = playerInputs.fwdAxis.z;

	this->playerInputs.rightAxis.x = playerInputs.rightAxis.x;
	this->playerInputs.rightAxis.y = playerInputs.rightAxis.y;
	this->playerInputs.rightAxis.z = playerInputs.rightAxis.z;
	this->playerInputs.cameraYaw = playerInputs.cameraYaw;
	
	this-> lastId = lastId;
	this->mouseXLook = mouseXLook;
}

NetworkObject::NetworkObject(GameObject& o, int id) : object(o)	{
	deltaErrors = 0;
	fullErrors  = 0;
	networkID   = id;
}

NetworkObject::~NetworkObject()	{
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