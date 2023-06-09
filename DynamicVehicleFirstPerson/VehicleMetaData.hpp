#pragma once
#include <inc/types.h>

enum class ESeatPosition {
    Left,
    Center,
    Right
};

class CVehicleMetaData {
public:
    CVehicleMetaData(Vehicle vehicle);

    void Update();

    Hash Model() { return mModel; }
    ESeatPosition GetSeatPosition() { return mSeatPosition; }
    Vector3 Acceleration() { return mAcceleration; }
    Vector3 AccelerationCentripetal() { return mAccelerationCentripetal; }
private:
    ESeatPosition getSeatPosition() const;
    Vector3 calculateAcceleration();
    Vector3 calculateAccelerationCentripetal();

    Vehicle mVehicle;

    Hash mModel = 0;
    ESeatPosition mSeatPosition = ESeatPosition::Center;

    Vector3 mVelocity{};
    Vector3 mAcceleration{};

    Vector3 mWorldVelocity{};
    Vector3 mAccelerationCentripetal{};
};
