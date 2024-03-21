#pragma once

#ifdef USEGL
#include <queue>
#include <mutex>
#include <functional>
#include <random>
#include "NetworkedGame.h"


namespace NCL::CSC8503
{
    struct SyncPlayerIdNameMapPacket;
}

namespace NCL::CSC8503
{
    struct ClientInitPacket;
}

namespace NCL::CSC8503
{
    struct SyncObjectStatePacket;
}

namespace NCL::CSC8503 {
    struct SyncInteractablePacket;
    struct ClientSyncItemSlotPacket;
}

namespace NCL{
    namespace CSC8503{
        struct DeltaPacket;
        class GameServer;
        class GameClient;
        class NetworkPlayer;

        struct FullPacket;
        struct ClientPlayerInputPacket;
        struct ClientSyncBuffPacket;
        struct AddPlayerScorePacket;
        struct ClientSyncLocalActiveSusCausePacket;
        struct ClientSyncLocalSusChangePacket;
        struct ClientSyncGlobalSusChangePacket;
        struct ClientSyncLocationActiveSusCausePacket;
        struct ClientSyncLocationSusChangePacket;
        struct AnnouncementSyncPacket;
        struct GuardSpotSoundPacket;

        class DebugNetworkedGame : public NetworkedGame{
        public:
            DebugNetworkedGame();
            ~DebugNetworkedGame();
            
            bool GetIsServer() const;
            bool PlayerWonGame() override;
            bool PlayerLostGame() override;
            const bool GetIsGameStarted() const;

            const int GetClientLastFullID() const;

            bool StartAsServer(const std::string& playerName);
            bool StartAsClient(char a, char b, char c, char d, const std::string& playerName);

            void UpdateGame(float dt) override;

            void SetIsGameStarted(bool isGameStarted, unsigned int seed = -1);
            void SetIsGameFinished(bool isGameFinished, int winningPlayerId);
            void StartLevel(const std::mt19937& levelSeed);

            void AddEventOnGameStarts(std::function<void()> event);

            void ReceivePacket(int type, GamePacket* payload, int source) override;
            void InitInGameMenuManager() override;

            void SendInteractablePacket(int networkObjectId, bool isOpen, int interactableItemType) const;
            void SendClientSyncItemSlotPacket(int playerNo, int invSlot, int inItem, int usageCount) const;
            void SendClientSyncBuffPacket(int playerNo, int buffType, bool toApply) const;
            void SendObjectStatePacket(int networkId, int state) const;
            void ClearNetworkGame();

            void SendClientSyncLocalActiveSusCausePacket(int playerNo, int activeSusCause, bool toApply) const;
            void SendClientSyncLocalSusChangePacket(int playerNo, int changedValue) const;
            void SendClientSyncGlobalSusChangePacket(int changedValue) const;
            void SendClientSyncLocationActiveSusCausePacket(int cantorPairedLocation, int activeSusCause, bool toApply) const;
            void SendClientSyncLocationSusChangePacket(int cantorPairedLocation, int changedValue) const;

            void SendAnnouncementSyncPacket(int annType, float time,int playerNo);

            void SendGuardSpotSoundPacket(int playerId) const;

            void SendPacketsThread();

            GameClient* GetClient() const;
            GameServer* GetServer() const;
            NetworkPlayer* GetLocalPlayer() const;

        protected:
            bool mIsGameStarted = false;
            bool mIsGameFinished = false;
            bool mIsServer = false;

            int mWinningPlayerId;
            int mLocalPlayerId;

            void UpdateAsServer(float dt);
            void UpdateAsClient(float dt);

            void BroadcastSnapshot(bool deltaFrame);
            void UpdateMinimumState();
            int GetPlayerPeerID(int peerId = -2);

            void SendStartGameStatusPacket(const std::string& seed = "") const;
            void SendFinishGameStatusPacket();
            
            void InitWorld(const std::mt19937& levelSeed);

            void HandleClientPlayerInput(ClientPlayerInputPacket* playerMovementPacket, int playerPeerID);

            void SpawnPlayers();

        	NetworkPlayer* AddPlayerObject(const Maths::Vector3& position, int playerNum);

            void HandleFullPacket(FullPacket* fullPacket);

            void HandleDeltaPacket(DeltaPacket* deltaPacket);

            void HandleClientPlayerInputPacket(ClientPlayerInputPacket* clientPlayerInputPacket, int playerPeerId);

            void HandleAddPlayerScorePacket(AddPlayerScorePacket* packet);

            void SyncPlayerList();

        	void SetItemsLeftToZero() override;

            void HandlePlayerEquippedItemChange(ClientSyncItemSlotPacket* packet) const;

            void HandleInteractablePacket(SyncInteractablePacket* packet) const;

        	void HandlePlayerBuffChange(ClientSyncBuffPacket* packet) const;

            void HandleObjectStatePacket(SyncObjectStatePacket* packet) const;

            void HandleLocalActiveSusCauseChange(ClientSyncLocalActiveSusCausePacket* packet) const;
            void HandleLocalSusChange(ClientSyncLocalSusChangePacket* packet) const;
            void HandleGlobalSusChange(ClientSyncGlobalSusChangePacket* packet) const;
            void HandleLocationActiveSusCauseChange(ClientSyncLocationActiveSusCausePacket* packet) const;
            void HandleLocationSusChange(ClientSyncLocationSusChangePacket* packet) const;

            void HandleAnnouncementSync(const AnnouncementSyncPacket* packet) const;

            void HandleGuardSpotSound(GuardSpotSoundPacket* packet) const;

            void AddToPlayerPeerNameMap(int playerId, const std::string& playerName);
            void HandleClientInitPacket(const ClientInitPacket* packet, int playerID);

            void WriteAndSendSyncPlayerIdNameMapPacket() const;
            void HandleSyncPlayerIdNameMapPacket(const SyncPlayerIdNameMapPacket* packet);

            void ShowPlayerList() const;

            std::vector<std::function<void()>> mOnGameStarts;

            int mNetworkObjectCache = 10;

            int mClientSideLastFullID;
            int mServerSideLastFullID;

            std::queue<GamePacket*> mPacketToSendQueue;
            std::mutex mPacketToSendQueueMutex;

            std::map<int, std::string> mPlayerPeerNameMap;
        private:
        };
    }
}
#endif