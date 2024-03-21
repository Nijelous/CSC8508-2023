#pragma once
#include "SuspicionMetre.h"
#include <vector>
#include "../NCLCoreClasses/Vector3.h"

using namespace NCL::Maths;

namespace SuspicionSystem
{
    const float MAX_NEARBY_DISTANCE = 5;
    struct CantorPair{

        CantorPair(){
            mValue = 0;
        }

        CantorPair(const int& inValue) {
            mValue = inValue;
        }

        CantorPair(Vector3 inPos){
            Vector3 shiftedPos;
            shiftedPos.x = int(int(inPos.x) > 0 ? int(inPos.x) * 2 : int(inPos.x) * -2 + 1);
            shiftedPos.z = int(int(inPos.z) > 0 ? int(inPos.z) * 2 : int(inPos.z) * -2 + 1);

            mValue = (shiftedPos.x + shiftedPos.z) * 
                     (shiftedPos.x + shiftedPos.z + 1) / 2 + shiftedPos.z;
        };

        bool operator==(const CantorPair& other) const {
            return mValue == other.mValue;
        }

        bool operator<(const CantorPair& other) const {
            return mValue < other.mValue;
        }   

        int operator*() const {
            return mValue;
        }

        operator int() const {
            return mValue;
        }

        CantorPair& operator=(int value) {
            mValue = value;
            return *this;
        }
        
        static Vector3 InverseCantorPair(CantorPair inPair){
            int w = int((sqrt(8 * inPair + 1) - 1) / 2);
            int t = (w * w + w) / 2;
            Vector3 shiftedPos;
            shiftedPos.z = inPair - t;
            shiftedPos.x = w - shiftedPos.z;
            Vector3 outPos;
            outPos.x = (int(shiftedPos.x) % 2 == 0 ? shiftedPos.x / 2 : -(shiftedPos.x / 2) );
            outPos.z = (int(shiftedPos.z) % 2 == 0 ? shiftedPos.z / 2 : -(shiftedPos.z / 2) );
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
        void SetMinLocationSusAmount(Vector3 pos, float susValue);

        void UpdateVec3LocationSusAmountMap();

        std::map<Vector3, float>* GetVec3LocationSusAmountMapPtr() {
            return (&mVec3LocationSusAmountMap);
        };

        SuspicionMetre::SusBreakpoint GetLocalSusMetreBreakpoint(Vector3 pos)
        {
            return SuspicionMetre::GetSusBreakpoint(GetLocationSusAmount(pos));
        }

        void SyncActiveSusCauses(const activeLocationSusCause& inCause, const int& pairedLocation, const bool& toApply);
        void SyncSusChange(const int& pairedLocation, const int& changedValue);
        void RemoveSusLocation(const Vector3 pos);

    private:

        std::map<const instantLocationSusCause, const float>  mInstantLocationSusCauseSeverityMap =
        {
            {singleSoundEmitted, 2}
        };

        std::map<const activeLocationSusCause, const float>  activeLocationSusCauseSeverityMap =
        {
            {continouousSound, 5}, {cameraLOS, 3}, {susPlayerNearby,2}, {passiveRecovery,-2}
        };

        std::map<Vector3, float> mVec3LocationSusAmountMap;
        std::map<CantorPair, float> mLocationSusAmountMap;
        std::map<CantorPair, std::vector<activeLocationSusCause>> mActiveLocationSusCauseMap;
        std::map<CantorPair, std::vector<activeLocationSusCause>> mActiveLocationlSusCausesToRemove;
        std::vector<CantorPair> locationsToClear;

        bool AddActiveLocationSusCause(activeLocationSusCause inCause, CantorPair pairedLocation);
        bool RemoveActiveLocationSusCause(activeLocationSusCause inCause, CantorPair pairedLocation);

        bool IsNearbySusLocation(CantorPair pairedLocation, CantorPair& nearbyPairedLocation) const;

        void ChangeSusLocationSusAmount(CantorPair pairedLocation, float amount);
        void AddNewLocation(CantorPair pairedLocation, float initSusAmount = 0.0f);

        bool IsActiveLocationsSusCause(activeLocationSusCause inCause, CantorPair pairedLocation);

        float Calculate2DDistance(Vector3 inPos1, Vector3 inPos2) const;
    
        void HandleActiveSusCauseNetworking(const activeLocationSusCause& inCause, const CantorPair& pairedLocation, const bool& toApply);
        void HandleSusChangeNetworking(const int& changedValue, const CantorPair& pairedLocation);

    };
}
