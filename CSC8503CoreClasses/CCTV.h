#pragma once
#include "GameObject.h"
#include "Pyramid.h"
#include "../CSC8503/NetworkPlayer.h"

namespace NCL {
	namespace CSC8503 {


		class CCTV : public GameObject {
		public:
			CCTV(const Vector3 base, const float baseL = 5,const std::string& name = ""){
				mBase = base;
				mBaseL = baseL;
			};
			~CCTV() {};

			void DrawDebugLines(const bool canSeePlayer);
			void UpdateObject(float dt) override;
			void GenerateViewPyramid() {
				const Vector3 thisPos = GetTransform().GetPosition();

				mViewPyramid = Pyramid(thisPos, mBase, mBaseL);
			}
			void SetPlayerObjectPtr(PlayerObject* playerObjectPtr) {
				mPlayerObject = playerObjectPtr;
			}
			void SetServerPlayersPtr(std::map<int, NetworkPlayer*>* serverPlayersPtr) {
				mServerPlayersPtr = serverPlayersPtr;
			}
			bool CanSeePlayer(PlayerObject* mPlayerObject);
			void OnPlayerSeen(PlayerObject* mPlayerObject);
			void OnPlayerNotSeen(PlayerObject* mPlayerObject);
		protected:
			Vector3 mBase;
			float mBaseL;
			Pyramid mViewPyramid;
			//SinglePlayer
			PlayerObject* mPlayerObject=nullptr;
			//Multiplayer
			std::map<int, NetworkPlayer*>* mServerPlayersPtr;
			std::map<int, bool> hadSeenPlayer;
		};
	}
}

