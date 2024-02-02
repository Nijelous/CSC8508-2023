#pragma once
#include <map>
#include "NetworkBase.h"
#include "TutorialGame.h"

namespace NCL
{
    namespace CSC8503
    {
        class GameServer;
        class GameClient;
        class NetworkPlayer;

        struct FullPacket;
        struct ClientPlayerInputPacket;
        struct AddPlayerScorePacket;
        
        class DebugNetworkedGame : public TutorialGame, public PacketReceiver
        {
        public:
            DebugNetworkedGame();
            ~DebugNetworkedGame();
            
            void StartAsServer();
            void StartAsClient(char a, char b, char c, char d);

            void UpdateGame(float dt) override;
            
            void SetIsGameStarted(bool isGameStarted);
            void StartLevel();

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

            void HandleFullPacket(FullPacket* fullPacket );

            void HandleAddPlayerScorePacket(AddPlayerScorePacket* packet);

            void SyncPlayerList();

            std::map<int, int> stateIDs;

            GameServer* thisServer;
            GameClient* thisClient;
            float timeToNextPacket;
            int packetsToSnapshot;

            std::vector<NetworkObject*> networkObjects;

            std::vector<int> playerList;
            std::map<int, NetworkPlayer*> serverPlayers;
            GameObject* localPlayer;

            int networkObjectCache = 10;
        private:
        };
    }
}
