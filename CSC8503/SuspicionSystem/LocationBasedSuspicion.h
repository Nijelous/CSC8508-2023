#pragma once
#include "SuspicionMetre.h"
#include <cmath>
#include <vector>

namespace SuspicionSystem
{
    float MAX_NEARBY_DISTANCE = 5;
    float DT_UNTIL_LOCATION_RECOVERY = 5;

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

        void AddInstantLocalSusCause(instantLocationSusCause inCause, float locationX, float locationZ);

        bool AddActiveLocationSusCause(activeLocationSusCause inCause, float locationX, float locationZ);
        bool RemoveActiveLocationSusCause(activeLocationSusCause inCause, float locationX, float locationZ);

        void Update(float dt);

        int GetLocationSusAmount(float locationX, float locationZ);

        SuspicionMetre::SusBreakpoint GetLocalSusMetreBreakpoint(float locationX, float locationZ)
        {
            return SuspicionMetre::GetSusBreakpoint(GetLocationSusAmount(locationX, locationZ));
        }

    private:

        std::map<instantLocationSusCause, float>  mInstantLocationSusCauseSeverityMap =
        {
            {singleSoundEmitted, 2}
        };

        std::map<activeLocationSusCause, float>  activeLocationSusCauseSeverityMap =
        {
            {continouousSound, 3}, {cameraLOS, 3}, {susPlayerNearby,2},
        };

        std::map<int, float> mLocationSusAmountMap;
        std::map<int, float> mLocationRecoveryCDMap;
        std::map<int, std::vector<activeLocationSusCause>> mActiveLocationSusCauseMap;

        bool RemoveActiveLocationSusCause(activeLocationSusCause inCause, int pairedLocation);

        bool IsNearbySusLocation(int pairedLocation, int& nearbyPairedLocation);

        void ChangeSusLocationSusAmount(int pairedLocation, float amount);
        void AddNewLocation(int pairedLocation, float initSusAmount = 0.0f);

        bool IsActiveLocationsSusCause(activeLocationSusCause inCause, int pairedLocation);

        float CalculateDistance(int x1, int x2, int y1, int y2);
        int CantorPair(int x, int y);
        void InverseCantorPair(int z, int& x, int& y);
    };
}
