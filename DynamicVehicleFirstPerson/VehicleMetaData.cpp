#include "VehicleMetaData.hpp"
#include "Util/Math.hpp"
#include <inc/natives.h>

CVehicleMetaData::CVehicleMetaData(Vehicle vehicle)
    : mVehicle(vehicle) {
    if (!ENTITY::DOES_ENTITY_EXIST(vehicle))
        return;

    mModel = ENTITY::GET_ENTITY_MODEL(vehicle);
    mIsRHD = readRHD();
    mVelocity = ENTITY::GET_ENTITY_SPEED_VECTOR(mVehicle, true);
}

void CVehicleMetaData::Update() {
    // Calculate values based on old values first
    mAcceleration = calculateAcceleration();
    mAccelerationCentripetal = calculateAccelerationCentripetal();

    // Then update values
    mVelocity = ENTITY::GET_ENTITY_SPEED_VECTOR(mVehicle, true);
    mWorldVelocity = ENTITY::GET_ENTITY_VELOCITY(mVehicle);
}

bool CVehicleMetaData::readRHD() const {
    if (!ENTITY::DOES_ENTITY_EXIST(mVehicle))
        return false;

    return ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(
        mVehicle,
        ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(
            mVehicle,
            ENTITY::GET_ENTITY_BONE_INDEX_BY_NAME(
                mVehicle,
                "seat_dside_f"
            )
        )
    ).x > 0.01f;
}

Vector3 CVehicleMetaData::calculateAcceleration() {
    auto velocity = ENTITY::GET_ENTITY_SPEED_VECTOR(mVehicle, true);
    return (velocity - mVelocity) / MISC::GET_FRAME_TIME();
}

Vector3 CVehicleMetaData::calculateAccelerationCentripetal() {
    Vector3 worldVelocity = ENTITY::GET_ENTITY_VELOCITY(mVehicle);
    Vector3 worldVelDelta = (worldVelocity - mWorldVelocity);

    Vector3 fwdVec = ENTITY::GET_ENTITY_FORWARD_VECTOR(mVehicle);
    Vector3 upVec = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(mVehicle, { 0.0f, 0.0f, 1.0f })
        - ENTITY::GET_ENTITY_COORDS(mVehicle, true);
    Vector3 rightVec = Cross(fwdVec, upVec);

    return Vector3 {
        -Dot(worldVelDelta, rightVec),
        Dot(worldVelDelta, fwdVec),
        Dot(worldVelDelta, upVec),
    } / MISC::GET_FRAME_TIME();
}
