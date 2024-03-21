#pragma once
#include "GameObject.h"
#include "Pyramid.h"
#include "../CSC8503/NetworkPlayer.h"
#include "PlayerObject.h"

namespace NCL {
	namespace CSC8503 {
		class CCTV : public GameObject {
		public:
			CCTV(const float baseL = 5, GameWorld* world = nullptr, const std::string& name = ""){
				mBaseL = baseL;
				mName = "CCTV";
				mWorld = world;
				
			};
			~CCTV() {
				hadSeenPlayer.clear();
			};

			void DrawDebugLines(const bool canSeePlayer);
			void UpdateObject(float dt) override;
			const void UpdateForPlayerObject(PlayerObject* playerObjectPtr, const float dt);
			void GenerateViewPyramid();
			Vector3 GetBase(float angle);
			void SetPlayerObjectPtr(PlayerObject* playerObjectPtr) {
				mPlayerObject = playerObjectPtr;
			}
			void SetInitAngle(float angle) {
				initAngle = angle;
			}
			const bool CanSeePlayer(PlayerObject* mPlayerObject) ;
			const bool PlayerInRaycast(PlayerObject* mPlayerObject) ;
			const void OnPlayerSeen(PlayerObject* mPlayerObject);
			const void OnPlayerNotSeen(PlayerObject* mPlayerObject);
			static void AngleToNormalisedCoords(float angle, float& x, float& y);
		protected:
			float mBaseL;
			Pyramid mViewPyramid;
			PlayerObject* mPlayerObject=nullptr;
			std::map<int, bool> hadSeenPlayer;
			float initAngle;
			float rotateAngle;
			GameWorld* mWorld;
		};
	}
}

