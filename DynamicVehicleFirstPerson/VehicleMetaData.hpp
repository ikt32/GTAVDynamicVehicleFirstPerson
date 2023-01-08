#pragma once
#include <inc/types.h>

class CVehicleMetaData {
public:
    CVehicleMetaData(Vehicle vehicle);

    void Update();

    Hash Model() { return mModel; }
    bool IsRHD() { return mIsRHD; }
    Vector3 Acceleration() { return mAcceleration; }
    Vector3 AccelerationCentripetal() { return mAccelerationCentripetal; }
private:
    bool readRHD() const;
    Vector3 calculateAcceleration();
    Vector3 calculateAccelerationCentripetal();

    Vehicle mVehicle;

    Hash mModel = 0;
    bool mIsRHD = false;

    Vector3 mVelocity{};
    Vector3 mAcceleration{};

    Vector3 mWorldVelocity{};
    Vector3 mAccelerationCentripetal{};
};
