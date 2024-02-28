#pragma once
#include <functional>
#include "NetworkBase.h"
#include "GameSceneManager.h"
#include "NetworkedGame.h"

namespace NCL::CSC8503
{
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
        struct AddPlayerScorePacket;

        class DebugNetworkedGame : public NetworkedGame{
        public:
            DebugNetworkedGame();
            ~DebugNetworkedGame();
            
            bool GetIsServer() const;

            const int GetClientLastFullID() const;

            void StartAsServer();
            void StartAsClient(char a, char b, char c, char d);

            void UpdateGame(float dt) override;

            void SetIsGameStarted(bool isGameStarted);
            void SetIsGameFinished(bool isGameFinished);
            void StartLevel();

            void AddEventOnGameStarts(std::function<void()> event);

            void ReceivePacket(int type, GamePacket* payload, int source) override;

            void SendClinentSyncItemSlotPacket(int playerNo, int invSlot, int inItem) const;

            GameClient* GetClient() const;
            GameServer* GetServer() const;
            NetworkPlayer* GetLocalPlayer() const;

        protected:
            bool mIsGameStarted = false;
            bool mIsGameFinished = false;
            bool mIsServer = false;

            void UpdateAsServer(float dt);
            void UpdateAsClient(float dt);

            void BroadcastSnapshot(bool deltaFrame);
            void UpdateMinimumState();
            int GetPlayerPeerID(int peerId = -2);

            void SendStartGameStatusPacket();
            void SendFinishGameStatusPacket();
            
            void InitWorld() override;

            void HandleClientPlayerInput(ClientPlayerInputPacket* playerMovementPacket, int playerPeerID);

            void SpawnPlayers();
            NetworkPlayer* AddPlayerObject(const Vector3& position, int playerNum);

            void HandleFullPacket(FullPacket* fullPacket);

            void HandleDeltaPacket(DeltaPacket* deltaPacket);

            void HandleClientPlayerInputPacket(ClientPlayerInputPacket* clientPlayerInputPacket, int playerPeerId);

            void HandleAddPlayerScorePacket(AddPlayerScorePacket* packet);

            void SyncPlayerList();
            void SetItemsLeftToZero() override;

            void HandlePlayerEquippedItemChange(ClientSyncItemSlotPacket* packet) const;


            std::vector<std::function<void()>> mOnGameStarts;

            int mNetworkObjectCache = 10;

            int mClientSideLastFullID;
            int mServerSideLastFullID;

        private:
        };
    }
}
