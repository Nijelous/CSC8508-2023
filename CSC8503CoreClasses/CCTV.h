#pragma once
#include "GameObject.h"
#include "Pyramid.h"
#include "../CSC8503/NetworkPlayer.h"

namespace NCL {
	namespace CSC8503 {
	

		class CCTV : public GameObject {
		public:
			CCTV(const float baseL = 5,const std::string& name = ""){
				mBaseL = baseL;
				mName == "CCTV";
			};
			~CCTV() {
			};

			void DrawDebugLines(const bool canSeePlayer);
			void UpdateObject(float dt) override;
			void GenerateViewPyramid();
			Vector3 GetBase(float angle);
			void SetPlayerObjectPtr(PlayerObject* playerObjectPtr) {
				mPlayerObject = playerObjectPtr;
			}
			bool CanSeePlayer(PlayerObject* mPlayerObject) const;
			const void OnPlayerSeen(PlayerObject* mPlayerObject);
			const void OnPlayerNotSeen(PlayerObject* mPlayerObject);
			void AngleToNormalisedCoords(float angle, float& x, float& y) const;
			
		protected:
			float mBaseL;
			Pyramid mViewPyramid;
			PlayerObject* mPlayerObject=nullptr;
			std::map<int, bool> hadSeenPlayer;
			float rotateAngle;
		};
	}
}

