#pragma once
#include "SuspicionMetre.h"
#include <cmath>
#include <vector>
#include "../NCLCoreClasses/Vector3.h"
#include "../NCLCoreClasses/Vector2.h"

using namespace NCL::Maths;

namespace SuspicionSystem
{
    const float MAX_NEARBY_DISTANCE = 5;
    const float DT_UNTIL_LOCATION_RECOVERY = 5;

    struct CantorPair{

        CantorPair(){
            mValue = 0;
        }

        CantorPair(Vector3 inPos){
            mValue = ((inPos.x + inPos.y) * (inPos.x + inPos.y + 1)) / 2 + inPos.y;
        };

        bool operator<(const CantorPair& other) const {
            return mValue < other.mValue;
        }
        
        static Vector3 InverseCantorPair(CantorPair inPair){
            int w = static_cast<int>((std::sqrt(8 * inPair.mValue + 1) - 1) / 2);
            int t = (w * w + w) / 2;

            Vector3 outPos;
            outPos.y = inPair.mValue - t;
            outPos.x = w - outPos.y;
            return outPos;
        }

        int mValue;
    };

    class LocationBasedSuspicion :
        public SuspicionMetre
    {
    public:
        const enum instantLocationSusCause
        {
            singleSoundEmitted
        };

        const enum activeLocationSusCause
        {
            continouousSound, cameraLOS, susPlayerNearby, passiveRecovery
        };

        void Init();

        void AddInstantLocalSusCause(instantLocationSusCause inCause, Vector3 pos);

        bool AddActiveLocationSusCause(activeLocationSusCause inCause, Vector3 pos);
        bool RemoveActiveLocationSusCause(activeLocationSusCause inCause, Vector3 pos);

        void Update(float dt);

        int GetLocationSusAmount(Vector3 pos);

        SuspicionMetre::SusBreakpoint GetLocalSusMetreBreakpoint(Vector3 pos)
        {
            return SuspicionMetre::GetSusBreakpoint(GetLocationSusAmount(pos));
        }

    private:

        std::map<instantLocationSusCause, float>  mInstantLocationSusCauseSeverityMap =
        {
            {singleSoundEmitted, 2}
        };

        std::map<activeLocationSusCause, float>  activeLocationSusCauseSeverityMap =
        {
            {continouousSound, 3}, {cameraLOS, 3}, {susPlayerNearby,2}, {passiveRecovery,-2}
        };

        std::map<CantorPair, float> mLocationSusAmountMap;
        std::map<CantorPair, std::vector<activeLocationSusCause>> mActiveLocationSusCauseMap;

        bool AddActiveLocationSusCause(activeLocationSusCause inCause, CantorPair pairedLocation);
        bool RemoveActiveLocationSusCause(activeLocationSusCause inCause, CantorPair pairedLocation);

        bool IsNearbySusLocation(CantorPair pairedLocation, CantorPair& nearbyPairedLocation);

        void ChangeSusLocationSusAmount(CantorPair pairedLocation, float amount);
        void AddNewLocation(CantorPair pairedLocation, float initSusAmount = 0.0f);

        bool IsActiveLocationsSusCause(activeLocationSusCause inCause, CantorPair pairedLocation);

        float Calculate2DDistance(Vector3 inPos1, Vector3 inPos2) const;
    };
}
