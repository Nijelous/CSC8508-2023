#ifdef USEGL
#pragma once
#include "GameObject.h"
#include "NetworkBase.h"
#include "NetworkState.h"
#include "../CSC8503/NetworkPlayer.h"

namespace NCL::CSC8503 {
	class GameObject;

	struct FullPacket : public GamePacket {
		int		objectID = -1;
		NetworkState fullState;

		FullPacket() {
			type = Full_State;
			size = sizeof(FullPacket) - sizeof(GamePacket);
		}
	};

	struct DeltaPacket : public GamePacket {
		int		fullID		= -1;
		int		objectID	= -1;
		char	pos[3];
		char	orientation[4];

		DeltaPacket() {
			type = Delta_State;
			size = sizeof(DeltaPacket) - sizeof(GamePacket);
		}
	};

	struct ClientPacket : public GamePacket {
		int		lastID;
		char	buttonstates[8];
		Vector3 cameraPosition;

		ClientPacket() {
			type = Received_State;
			size = sizeof(ClientPacket) - sizeof(GamePacket);
			cameraPosition = Vector3(0,0,0);
		}
	};
	
	struct SyncPlayerListPacket : public GamePacket {
		int playerList[4];
		
		SyncPlayerListPacket(std::vector<int>& serverPlayers);
		void SyncPlayerList(std::vector<int>& clientPlayerList) const;
	};

	struct GameStartStatePacket : public GamePacket {
		bool isGameStarted = false;
		std::string levelSeed;
		GameStartStatePacket(bool val, const std::string& seed);
	};

	struct GameEndStatePacket : public GamePacket{
		bool isGameEnded = false;
		int winningPlayerId;

		GameEndStatePacket(bool val, int winningPlayerId);
	};

	struct ClientPlayerInputPacket : public GamePacket{
		int lastId;
		PlayerInputs playerInputs;
		float mouseXLook = 0.0f;
		
		ClientPlayerInputPacket(int lastId,  const PlayerInputs& playerInputs);
	};

	struct ClientUseItemPacket : public GamePacket {
		int objectID;
		int playerID;

		ClientUseItemPacket(int objectID, int playerID);
	};

	struct ClientSyncBuffPacket : public GamePacket {
		int playerID;
		int buffID;
		bool toApply;

		ClientSyncBuffPacket(int playerID, int buffID, bool toApply);
	};

	struct ClientSyncItemSlotUsagePacket : public GamePacket {
		int playerID;
		int firstItemUsage;
		int secondItemUsage;

		ClientSyncItemSlotUsagePacket(int playerID, int firstItemUsage, int secondItemUsage);
	};

	struct ClientSyncItemSlotPacket : public GamePacket {
		int playerID;
		int slotId;
		int equippedItem;
		int usageCount;

		ClientSyncItemSlotPacket(int playerID, int slotId, int equippedItem, int usageCount);
	};

	struct ClientSyncLocalActiveSusCausePacket : public GamePacket {
		int playerID;
		int activeLocalSusCauseID;
		bool toApply;

		ClientSyncLocalActiveSusCausePacket(int playerID, int activeLocalSusCauseID, bool toApply);
	};

	struct ClientSyncLocalSusChangePacket : public GamePacket {
		int playerID;
		int changedValue;

		ClientSyncLocalSusChangePacket(int playerID, int changedValue);
	};

	struct ClientSyncGlobalSusChangePacket : public GamePacket {
		int changedValue;

		ClientSyncGlobalSusChangePacket(int changedValue);
	};

	struct ClientSyncLocationActiveSusCausePacket : public GamePacket {
		int cantorPairedLocation;
		int activeLocationSusCauseID;
		bool toApply;

		ClientSyncLocationActiveSusCausePacket(int playerID, int activeLocationSusCauseID, bool toApply);
	};

	struct ClientSyncLocationSusChangePacket : public GamePacket {
		int cantorPairedLocation;
		int changedValue;

		ClientSyncLocationSusChangePacket(int cantorPairedLocation, int changedValue);
	};

	struct SyncInteractablePacket : public GamePacket {
		int networkObjId;
		bool isOpen;
		int interactableItemType;

		SyncInteractablePacket(int networkObjectId, bool isOpen, int interactableItemType);
	};

	struct SyncObjectStatePacket : public GamePacket {
		int networkObjId;
		int objectState;

		SyncObjectStatePacket(int networkObjId, int objectState);
	};

	struct AnnouncementSyncPacket : public GamePacket {
		int annType;
		float time;
		int playerNo;
		AnnouncementSyncPacket(int annType, float time, int playerNo);
	};

	struct ClientInitPacket : public GamePacket {
		std::string playerName;

		ClientInitPacket(const std::string& playerName);
	};

	struct SyncPlayerIdNameMapPacket : public GamePacket {
		int playerIds[4] = { -1 , -1, -1,-1 };
		std::string playerNames[4];

		SyncPlayerIdNameMapPacket(const std::map<int, string>& playerIdNameMap);
	};

	struct GuardSpotSoundPacket : public GamePacket {
		int playerId;

		GuardSpotSoundPacket(const int playerId);
	};

	class NetworkObject {
	public:
		NetworkObject(GameObject& o, int id);
		virtual ~NetworkObject();

		//Called by clients
		virtual bool ReadPacket(GamePacket& p);
		//Called by servers
		virtual bool WritePacket(GamePacket** p, bool deltaFrame, int stateID);

		GameObject& GetGameObject() { return object; }

		void SetGameObject(GameObject& obj) const { object = obj; }

		int GetnetworkID() { return networkID; }

		void UpdateStateHistory(int minID);

		NetworkState& GetLatestNetworkState();

	protected:

		bool GetNetworkState(int frameID, NetworkState& state);

		virtual bool ReadDeltaPacket(DeltaPacket &p);
		virtual bool ReadFullPacket(FullPacket &p);

		virtual bool WriteDeltaPacket(GamePacket**p, int stateID);
		virtual bool WriteFullPacket(GamePacket**p);

		GameObject& object;

		NetworkState lastFullState;

		std::vector<NetworkState> stateHistory;

		int deltaErrors;
		int fullErrors;

		int networkID;
	};
}
#endif