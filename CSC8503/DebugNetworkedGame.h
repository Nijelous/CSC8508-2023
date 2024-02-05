#pragma once
#include <map>
#include "NetworkBase.h"
#include "TutorialGame.h"

namespace NCL{
    namespace CSC8503{
        class GameServer;
        class GameClient;
        class NetworkPlayer;

        struct FullPacket;
        struct ClientPlayerInputPacket;
        struct AddPlayerScorePacket;

        class DebugNetworkedGame : public TutorialGame, public PacketReceiver{
        public:
            DebugNetworkedGame();
            ~DebugNetworkedGame();

            void StartAsServer();
            void StartAsClient(char a, char b, char c, char d);

            void UpdateGame(float dt) override;

            void SetIsGameStarted(bool isGameStarted);
            void StartLevel();

            void AddEventOnGameStarts(std::function<void()> event);

            void ReceivePacket(int type, GamePacket* payload, int source) override;

            GameClient* GetClient() const;
            GameServer* GetServer() const;

        protected:
            bool isClientConnectedToServer = false;
            bool isGameStarted = false;

            void UpdateAsServer(float dt);
            void UpdateAsClient(float dt);

            void BroadcastSnapshot(bool deltaFrame);
            void UpdateMinimumState();
            int GetPlayerPeerID(int peerId = -2);

            void SendGameStatusPacket();
            void InitWorld() override;

            void HandleClientPlayerInput(ClientPlayerInputPacket* playerMovementPacket, int playerPeerID);

            void SpawnPlayers();
            NetworkPlayer* AddPlayerObject(const Vector3& position, int playerNum);

            void HandleFullPacket(FullPacket* fullPacket);

            void HandleAddPlayerScorePacket(AddPlayerScorePacket* packet);

            void SyncPlayerList();

            std::vector<std::function<void()>> mOnGameStarts;

            std::map<int, int> mStateIDs;

            GameServer* mThisServer;
            GameClient* mThisClient;
            float mTimeToNextPacket;
            int mPacketsToSnapshot;

            std::vector<NetworkObject*> mNetworkObjects;

            std::vector<int> mPlayerList;
            std::map<int, NetworkPlayer*> mServerPlayers;
            GameObject* mLocalPlayer;

            int mNetworkObjectCache = 10;

        private:
        };
    }
}
