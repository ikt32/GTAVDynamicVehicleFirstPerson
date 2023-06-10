#include "FPVScript.hpp"

#include "Util/Enums.hpp"
#include "Util/Math.hpp"
#include "Util/ScriptUtils.hpp"
#include "Util/Strings.hpp"
#include "Util/UI.hpp"

#include "Memory/MemoryAccess.hpp"
#include "Memory/VehicleExtensions.hpp"

#include <inc/enums.h>
#include <inc/main.h>
#include <inc/natives.h>
#include <format>
#include <map>

using std::to_underlying;

CFPVScript::CFPVScript(const std::shared_ptr<CScriptSettings>& settings,
    const std::shared_ptr<CShakeData>& shakeData,
    std::vector<CConfig>& configs)
    : mSettings(settings)
    , mShakeData(shakeData)
    , mConfigs(configs)
    , mVehicle(0)
    , mVehicleData(mVehicle)
    , mLookResetTimer(500) {
    // < VER_1_0_1290_1_STEAM
    if (getGameVersion() < 38) {
        mFpvCamOffsetXOffset = 0x428;
    }

    mPerlinNoise = std::make_unique<PerlinNoise>();
}

void CFPVScript::UpdateActiveConfig() {
    if (mConfigs.empty()) {
        // Should NOT occur, like, ever, but still.
        return;
    }
    if (!ENTITY::DOES_ENTITY_EXIST(mVehicle)) {
        mActiveConfig = &mConfigs[0];
        return;
    }

    Hash model = ENTITY::GET_ENTITY_MODEL(mVehicle);
    std::string plate = VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT(mVehicle);

    // First pass - match model and plate
    auto foundConfig = std::find_if(mConfigs.begin(), mConfigs.end(), [&](const CConfig& config) {
        bool modelMatch = config.ModelHash == model;
    bool plateMatch = StrUtil::Strcmpwi(config.Plate, plate);
    return modelMatch && plateMatch;
        });

    // second pass - match model with any plate
    if (foundConfig == mConfigs.end()) {
        foundConfig = std::find_if(mConfigs.begin(), mConfigs.end(), [&](const CConfig& config) {
            bool modelMatch = config.ModelHash == model;
        bool plateMatch = config.Plate.empty();
        return modelMatch && plateMatch;
            });
    }

    // third pass - use default
    if (foundConfig == mConfigs.end()) {
        mActiveConfig = &mConfigs[0];
    }
    else {
        mActiveConfig = &*foundConfig;
    }
}

void CFPVScript::Tick() {
    if (mActiveConfig) {
        update();
    }
    else {
        // mActiveConfig should never be nullptr
        Cancel();
        UpdateActiveConfig();
    }
}

void CFPVScript::Cancel() {
    if (CAM::DOES_CAM_EXIST(mHandle)) {
        CAM::RENDER_SCRIPT_CAMS(false, false, 0, true, false, 0);
        CAM::SET_CAM_ACTIVE(mHandle, false);
        CAM::DESTROY_CAM(mHandle, false);
        mHandle = -1;
        if (!mSettings->Debug.DisableRemoveHead) {
            hideHead(false);
        }
        HUD::UNLOCK_MINIMAP_ANGLE();
        GRAPHICS::SET_PARTICLE_FX_CAM_INSIDE_VEHICLE(false);
    }

    mRotation = {};
    mLookAcc = {};

    mInertiaDirectionLookAngle = 0.0f;
    mInertiaMove = {};
    mInertiaPitch = 0.0f;
    mDynamicPitch = 0.0f;

    mAverageAccel = 0.0f;
}


void CFPVScript::update() {
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);
    Hash model = ENTITY::GET_ENTITY_MODEL(vehicle);

    if (mVehicle != vehicle) {
        mVehicle = vehicle;
        UpdateActiveConfig();
        if (Util::VehicleAvailable(vehicle, playerPed)) {
            mVehicleData = CVehicleMetaData(vehicle);
        }
    }

    if (!Util::VehicleAvailable(vehicle, playerPed) ||
        !mSettings->Main.Enable ||
        !mActiveConfig ||
        !mActiveConfig->Enable) {
        Cancel();
        return;
    }

    mVehicleData.Update();

    bool fpv = CAM::GET_FOLLOW_VEHICLE_CAM_VIEW_MODE() == 4;
    bool hasControl = PLAYER::IS_PLAYER_CONTROL_ON(PLAYER::PLAYER_ID()) &&
        PED::IS_PED_SITTING_IN_VEHICLE(playerPed, vehicle);

    bool aiming = PAD::IS_CONTROL_PRESSED(2, ControlVehicleAim);

    // Don't check for aiming in air vehicles
    if (VEHICLE::IS_THIS_MODEL_A_PLANE(model) ||
        VEHICLE::IS_THIS_MODEL_A_HELI(model)) {
        aiming = false;
    }

    // Bikes use different seat bones
    bool bikeSeat = false;
    if (VEHICLE::IS_THIS_MODEL_A_BIKE(model) ||
        VEHICLE::IS_THIS_MODEL_A_QUADBIKE(model) ||
        VEHICLE::IS_THIS_MODEL_A_BICYCLE(model)) {
        bikeSeat = true;
    }

    if (!fpv || !hasControl || aiming) {
        Cancel();
        return;
    }

    PAD::DISABLE_CONTROL_ACTION(0, eControl::ControlVehicleCinCam, true);

    // Initialize camera
    if (mHandle == -1) {
        init();

        CAM::SET_CAM_ACTIVE(mHandle, true);
        CAM::SET_CAM_IS_INSIDE_VEHICLE(mHandle, true);
        GRAPHICS::SET_PARTICLE_FX_CAM_INSIDE_VEHICLE(true);
        CAM::RENDER_SCRIPT_CAMS(true, false, 0, true, false, 0);
    }
    CAM::SET_SCRIPTED_CAMERA_IS_FIRST_PERSON_THIS_FRAME(true);

    bool lookingIntoGlass = false;
    if (MT::LookingLeft() || MT::LookingRight() || MT::LookingBack()) {
        // Manual Transmission wheel keys
        updateWheelLook(lookingIntoGlass);
    }
    else if (PAD::IS_USING_KEYBOARD_AND_MOUSE(2) == TRUE) {
        // Mouse input
        updateMouseLook(lookingIntoGlass);
    }
    else {
        // Controller input
        updateControllerLook(lookingIntoGlass);
    }

    const auto& mount = mActiveConfig->Mount[mActiveConfig->CamIndex];
    const CConfig::SMovement& movement = mount.Movement;
    const CConfig::SDoF& dof = mount.DoF;

    if (movement.Follow) {
        updateRotationCameraMovement(movement);
        updateLongitudinalCameraMovement(movement);
        updateLateralCameraMovement(movement);
        updateVerticalCameraMovement(movement);
        updatePitchCameraMovement(movement);
    }

    if (dof.Enable) {
        updateDoF(dof);
    }

    if (mSettings->Debug.NearClip.Override) {
        CAM::SET_CAM_NEAR_CLIP(mHandle, mSettings->Debug.NearClip.Distance);
    }
    else if (!mHeadRemoved) {
        // FPV driving gameplay is 0.149
        // Add 2.6cm so the head model is entirely clipped out
        CAM::SET_CAM_NEAR_CLIP(mHandle, 0.175f);
    }
    else {
        // Walking gameplay near clip is 0.05.
        CAM::SET_CAM_NEAR_CLIP(mHandle, 0.01f);
    }
    // 10km in city, 15km outside
    CAM::SET_CAM_FAR_CLIP(mHandle, 12500.0f);

    int index = 0xFFFF;
    uintptr_t pModelInfo = Memory::GetModelInfo(model, &index);

    // offset from seat?
    // These offsets don't seem very version-sturdy. Oh well, hope R* doesn't knock em over.
    Vector3 camSeatOffset;
    camSeatOffset.x = *reinterpret_cast<float*>(pModelInfo + mFpvCamOffsetXOffset);
    camSeatOffset.y = *reinterpret_cast<float*>(pModelInfo + mFpvCamOffsetXOffset + 4);
    camSeatOffset.z = *reinterpret_cast<float*>(pModelInfo + mFpvCamOffsetXOffset + 8);
    float rollbarOffset = 0.0f;

    if (VEHICLE::GET_VEHICLE_MOD(vehicle, eVehicleMod::VehicleModFrame) != -1)
        rollbarOffset = *reinterpret_cast<float*>(pModelInfo + mFpvCamOffsetXOffset + 0x30);

    int seatBoneIdx = ENTITY::GET_ENTITY_BONE_INDEX_BY_NAME(vehicle,
        bikeSeat ? "seat_f" : "seat_dside_f");

    Vector3 seatCoords;
    Vector3 seatOffset;

    if (seatBoneIdx != -1) {
        seatCoords = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(
            vehicle, ENTITY::GET_ENTITY_BONE_INDEX_BY_NAME(
                vehicle, bikeSeat ? "seat_f" : "seat_dside_f"));
        seatOffset = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(
            vehicle, seatCoords);
    }
    else {
        seatCoords = ENTITY::GET_ENTITY_COORDS(vehicle, true);
        seatOffset = {};
    }

    if (bikeSeat) {
        Vector3 headBoneCoord = PED::GET_PED_BONE_COORDS(playerPed, 0x796E, {});
        Vector3 headBoneOff = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(
            playerPed, headBoneCoord);
        // SKEL_Spine_Root
        Vector3 spinebaseCoord = PED::GET_PED_BONE_COORDS(playerPed, 0xe0fd, {});
        Vector3 spinebaseOff = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(
            playerPed, spinebaseCoord);
        Vector3 offHead = headBoneOff - spinebaseOff;

        camSeatOffset = camSeatOffset + offHead;
    }

    float pitch = mount.Pitch;
    float fov = mount.FOV;

    Vector3 leanOffset = getLeanOffset(lookingIntoGlass);

    Vector3 shakeInfo{};
    if (mount.Movement.ShakeSpeed > 0.0f) {
        shakeInfo = getShakeFromSpeed();
    }

    if (mount.Movement.ShakeTerrain > 0.0f) {
        shakeInfo = shakeInfo + getShakeFromTerrain();
    }

    switch (mount.MountPoint) {
        case CConfig::EMountPoint::Ped: {
            // 0x796E skel_head id
            CAM::ATTACH_CAM_TO_PED_BONE(mHandle, playerPed, 0x796E, {
                mount.OffsetSide + leanOffset.x + mInertiaMove.x + shakeInfo.x,
                mount.OffsetForward + leanOffset.y + mInertiaMove.y,
                mount.OffsetHeight + leanOffset.z + mInertiaMove.z + shakeInfo.y
                }, true);
            break;
        }
        case CConfig::EMountPoint::Vehicle:
        default:
        {
            CAM::ATTACH_CAM_TO_ENTITY(mHandle, vehicle, {
                seatOffset.x + camSeatOffset.x + mount.OffsetSide + leanOffset.x + mInertiaMove.x + shakeInfo.x,
                seatOffset.y + camSeatOffset.y + mount.OffsetForward + leanOffset.y + mInertiaMove.y,
                seatOffset.z + camSeatOffset.z + mount.OffsetHeight + leanOffset.z + mInertiaMove.z + rollbarOffset + shakeInfo.y
                }, true);
            break;
        }
    }

    auto rot = ENTITY::GET_ENTITY_ROTATION(vehicle, 0);
    Vector3 horizonLockRotation = getHorizonLockRotation();

    float rollPitchComp = sin(deg2rad(mRotation.z)) * rot.y;
    float pitchLookComp = 0.0f;
    float rollLookComp = 0.0f;
    if (!mount.HorizonLock.Lock) {
        pitchLookComp = -rot.x * 2.0f * abs(mRotation.z) / 180.0f;
        rollLookComp = -rot.y * 2.0f * abs(mRotation.z) / 180.0f;
    }

    CAM::SET_CAM_ROT(
        mHandle, {
            rot.x + mRotation.x + pitch + pitchLookComp + rollPitchComp + mInertiaPitch - horizonLockRotation.x,
            rot.y + rollLookComp + horizonLockRotation.y + shakeInfo.z,
            rot.z + mRotation.z - mInertiaDirectionLookAngle
        },
        0);

    CAM::SET_CAM_FOV(mHandle, fov);

    float minimapAngle = rot.z + mRotation.z - mInertiaDirectionLookAngle;
    if (minimapAngle > 360.0f) minimapAngle = minimapAngle - 360.0f;
    if (minimapAngle < 0.0f) minimapAngle = minimapAngle + 360.0f;

    HUD::LOCK_MINIMAP_ANGLE(static_cast<int>(minimapAngle));
}

void CFPVScript::init() {
    auto cV = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(mVehicle, { 0.0f, 2.0f, 0.5f });
    mHandle = CAM::CREATE_CAM_WITH_PARAMS(
        "DEFAULT_SCRIPTED_CAMERA",
        cV,
        {},
        mActiveConfig->Mount[mActiveConfig->CamIndex].FOV, 1, 2);

    VEHICLE::SET_CAR_HIGH_SPEED_BUMP_SEVERITY_MULTIPLIER(0.0f);

    if (!mSettings->Debug.DisableRemoveHead) {
        hideHead(true);
    }

    mCumTimeSpeed = 0.0;
    mCumTimeTerrain = 0.0;
}

void CFPVScript::hideHead(bool remove) {
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    if (Dismemberment::Available()) {
        if (remove) {
            Dismemberment::AddBoneDraw(playerPed, 0x796E, -1);
            mHeadRemoved = true;
        }
        else {
            Dismemberment::RemoveBoneDraw(playerPed);
            mHeadRemoved = false;
        }
    }

    if (!mSettings->Debug.DisableRemoveProps) {
        if (remove) {
            savedHeadProp = PED::GET_PED_PROP_INDEX(playerPed, to_underlying(ePedPropPosition::AnchorHead));
            savedHeadPropTx = PED::GET_PED_PROP_TEXTURE_INDEX(playerPed, to_underlying(ePedPropPosition::AnchorHead));

            savedEyesProp = PED::GET_PED_PROP_INDEX(playerPed, to_underlying(ePedPropPosition::AnchorEyes));
            savedEyesPropTx = PED::GET_PED_PROP_TEXTURE_INDEX(playerPed, to_underlying(ePedPropPosition::AnchorEyes));

            PED::CLEAR_PED_PROP(playerPed, to_underlying(ePedPropPosition::AnchorHead));
            PED::CLEAR_PED_PROP(playerPed, to_underlying(ePedPropPosition::AnchorEyes));
        }
        else {
            if (savedHeadProp != -1)
                PED::SET_PED_PROP_INDEX(playerPed, to_underlying(ePedPropPosition::AnchorHead), savedHeadProp, savedHeadPropTx, true);
            if (savedEyesProp != -1)
                PED::SET_PED_PROP_INDEX(playerPed, to_underlying(ePedPropPosition::AnchorEyes), savedEyesProp, savedEyesPropTx, true);
        }
    }
}

// We generally want to look back through the center of the car, but follow the direction we already look into.
// If centered (motorcycle), look back over right shoulder only if already looking right.
// Otherwise, look back over left shoulder by default, as we usually drive on the right.
float CFPVScript::getRearLookAngle(ESeatPosition seatPosition, float lookLeftRight, float maxAngle) {
    // No pre-existing input: Follow seat position
    if (abs(lookLeftRight) < 0.05f) {
        if (seatPosition == ESeatPosition::Left) {
            return -maxAngle;
        }
        else if (seatPosition == ESeatPosition::Right) {
            return maxAngle;
        }
    }

    // Pre-existing input: Follow existing looking direction.
    // This is also the default for motorcycles/centered seats.
    if (lookLeftRight >= 0.05f) {
        return -maxAngle;
    }
    else {
        return maxAngle;
    }
}

void CFPVScript::updateControllerLook(bool& lookingIntoGlass) {
    float lookLeftRight = PAD::GET_CONTROL_NORMAL(0, eControl::ControlLookLeftRight);
    float lookUpDown = PAD::GET_CONTROL_NORMAL(0, eControl::ControlLookUpDown);

    auto seatPosition = mVehicleData.GetSeatPosition();
    if (seatPosition != ESeatPosition::Center &&
        mVehicleData.IsDriverWindowPresent()) {
        if (seatPosition == ESeatPosition::Right && lookLeftRight > 0.01f) {
            lookingIntoGlass = true;
        }
        if (seatPosition == ESeatPosition::Left && lookLeftRight < -0.01f) {
            lookingIntoGlass = true;
        }
    }

    float maxAngle = lookingIntoGlass ? 135.0f : 179.0f;

    if (lookingIntoGlass) {
        if (abs(lookLeftRight * 179.0f) > maxAngle) {
            lookLeftRight = sgn(lookLeftRight) * (maxAngle / 179.0f);
        }
    }
    mRotation.x = lerp(mRotation.x, 90.0f * -lookUpDown,
        1.0f - pow(mActiveConfig->Look.LookTime, MISC::GET_FRAME_TIME()));

    if (PAD::GET_CONTROL_NORMAL(0, eControl::ControlVehicleLookBehind) != 0.0f) {
        float lookBackAngle = getRearLookAngle(seatPosition, lookLeftRight, maxAngle);
        mRotation.z = lerp(mRotation.z, lookBackAngle,
            1.0f - pow(mActiveConfig->Look.LookTime, MISC::GET_FRAME_TIME()));
    }
    else {
        // Manual look
        mRotation.z = lerp(mRotation.z, 179.0f * -lookLeftRight,
            1.0f - pow(mActiveConfig->Look.LookTime, MISC::GET_FRAME_TIME()));
    }
}

void CFPVScript::updateMouseLook(bool& lookingIntoGlass) {
    float lookLeftRight =
        PAD::GET_CONTROL_NORMAL(0, eControl::ControlLookLeftRight) * mActiveConfig->Look.MouseSensitivity;
    float lookUpDown =
        PAD::GET_CONTROL_NORMAL(0, eControl::ControlLookUpDown) * mActiveConfig->Look.MouseSensitivity;
    bool lookBehind = PAD::GET_CONTROL_NORMAL(0, eControl::ControlVehicleLookBehind) != 0.0f;

    auto seatPosition = mVehicleData.GetSeatPosition();
    if (seatPosition != ESeatPosition::Center &&
        mVehicleData.IsDriverWindowPresent()) {
        if (seatPosition == ESeatPosition::Right && mLookAcc.x > 0.01f) {
            lookingIntoGlass = true;
        }
        if (seatPosition == ESeatPosition::Left && mLookAcc.x < -0.01f) {
            lookingIntoGlass = true;
        }
    }

    float maxAngle = lookingIntoGlass ? 135.0f : 179.0f;

    // Re-center on no input
    if (lookLeftRight != 0.0f || lookUpDown != 0.0f) {
        mLookResetTimer.Reset(mActiveConfig->Look.MouseCenterTimeout);
    }

    float speed = ENTITY::GET_ENTITY_SPEED(mVehicle);
    if (mLookResetTimer.Expired() && speed > 1.0f && !lookBehind) {
        mLookAcc.y = lerp(mLookAcc.y, 0.0f,
            1.0f - pow(mActiveConfig->Look.MouseLookTime, MISC::GET_FRAME_TIME()));
        mLookAcc.x = lerp(mLookAcc.x, 0.0f,
            1.0f - pow(mActiveConfig->Look.MouseLookTime, MISC::GET_FRAME_TIME()));
    }
    else {
        mLookAcc.y += lookUpDown;

        if (lookingIntoGlass) {
            if (sgn(lookLeftRight) != sgn(mLookAcc.x) || abs(mRotation.z) + abs(lookLeftRight * 179.0f) < maxAngle) {
                mLookAcc.x += lookLeftRight;
            }

            if (abs(mLookAcc.x * 179.0f) > maxAngle) {
                mLookAcc.x = sgn(mLookAcc.x) * (maxAngle / 179.0f);
            }
        }
        else {
            mLookAcc.x += lookLeftRight;
        }

        mLookAcc.y = std::clamp(mLookAcc.y, -1.0f, 1.0f);
        mLookAcc.x = std::clamp(mLookAcc.x, -1.0f, 1.0f);
    }

    mRotation.x = lerp(mRotation.x, 90 * -mLookAcc.y,
        1.0f - pow(mActiveConfig->Look.MouseLookTime, MISC::GET_FRAME_TIME()));

    // Override any mRotation.z changes while looking back
    if (lookBehind) {
        float lookBackAngle = getRearLookAngle(seatPosition, -mRotation.z, maxAngle);
        mRotation.z = lerp(mRotation.z, lookBackAngle,
            1.0f - pow(mActiveConfig->Look.MouseLookTime, MISC::GET_FRAME_TIME()));
    }
    else {
        mRotation.z = lerp(mRotation.z, 179.0f * -mLookAcc.x,
            1.0f - pow(mActiveConfig->Look.MouseLookTime, MISC::GET_FRAME_TIME()));
    }
}

void CFPVScript::updateWheelLook(bool& lookingIntoGlass) {
    if ((mMTLookRightPrev && MT::LookingRight()) &&
        (!mMTLookLeftPrev && MT::LookingLeft())) {
        // LookRight was pressed already, and LookLeft was just pressed
        mMTLookBackRightShoulder = true;
    }
    if (!MT::LookingLeft() ||
        !MT::LookingRight()) {
        // Any button released, stop caring about this
        mMTLookBackRightShoulder = false;
    }

    if (MT::LookingLeft() && MT::LookingRight() || MT::LookingBack()) {
        auto seatPosition = mVehicleData.GetSeatPosition();
        bool driverWindowPresent = mVehicleData.IsDriverWindowPresent();
        if (driverWindowPresent) {
            if (seatPosition == ESeatPosition::Right && mMTLookBackRightShoulder) {
                lookingIntoGlass = true;
            }
            if (seatPosition == ESeatPosition::Left && !mMTLookBackRightShoulder) {
                lookingIntoGlass = true;
            }
        }

        float maxAngle = lookingIntoGlass ? 135.0f : 179.0f;
        float lookBackAngle = mMTLookBackRightShoulder ? -maxAngle : maxAngle;
        mRotation.z = lerp(mRotation.z, lookBackAngle,
            1.0f - pow(mActiveConfig->Look.MouseLookTime, MISC::GET_FRAME_TIME()));
    }
    else {
        float angle;
        if (MT::LookingLeft()) {
            angle = 90.0f;
        }
        else {
            angle = -90.0f;
        }
        mRotation.z = lerp(mRotation.z, angle,
            1.0f - pow(mActiveConfig->Look.MouseLookTime, MISC::GET_FRAME_TIME()));
    }


    mMTLookLeftPrev = MT::LookingLeft();
    mMTLookRightPrev = MT::LookingRight();
}

void CFPVScript::updateRotationCameraMovement(const CConfig::SMovement& movement) {
    Vector3 speedVector = ENTITY::GET_ENTITY_SPEED_VECTOR(mVehicle , true);

    Vector3 target = Normalize(speedVector);
    float travelDir = atan2(target.y, target.x) - static_cast<float>(M_PI) / 2.0f;
    if (travelDir > static_cast<float>(M_PI) / 2.0f) {
        travelDir -= static_cast<float>(M_PI);
    }
    if (travelDir < -static_cast<float>(M_PI) / 2.0f) {
        travelDir += static_cast<float>(M_PI);
    }

    Vector3 rotationVelocity = ENTITY::GET_ENTITY_ROTATION_VELOCITY(mVehicle);

    float velComponent = travelDir * movement.RotationDirectionMult;
    float rotComponent = rotationVelocity.z * movement.RotationRotationMult;
    float rotMax = deg2rad(movement.RotationMaxAngle);
    float totalMove = std::clamp(velComponent + rotComponent,
        -rotMax,
        rotMax);
    float newAngle = -rad2deg(totalMove);

    if (speedVector.y < 3.0f) {
        newAngle = map(speedVector.y, 0.0f, 3.0f, 0.0f, newAngle);
        newAngle = std::clamp(newAngle, 0.0f, newAngle);
    }

    bool isPlane = VEHICLE::IS_THIS_MODEL_A_PLANE(mVehicleData.Model());
    bool isHeli = VEHICLE::IS_THIS_MODEL_A_HELI(mVehicleData.Model());
    bool isHover = VExt::GetHoverTransformRatio(mVehicle) > 0.0f;
    // 0.0f: Forward, 1.0f: Vertical
    // G_VER_1_0_1180_2_STEAM = 36
    bool isAirHover = getGameVersion() >= 36 && (isPlane || isHeli) &&
        VEHICLE::GET_VEHICLE_FLIGHT_NOZZLE_POSITION(mVehicle) > 0.5f;

    if (isHeli || isHover || isAirHover) {
        newAngle = 0.0f;
    }

    mInertiaDirectionLookAngle = lerp(mInertiaDirectionLookAngle, newAngle,
        1.0f - pow(0.000001f, MISC::GET_FRAME_TIME()));
}

float CFPVScript::getMovementLerpFactor(const CConfig::SMovement& movement) const {
    float baseRoughnessExp = -3.0f;
    float roughnessExp = baseRoughnessExp - movement.Roughness;
    float roughness = pow(10.0f, roughnessExp);
    return 1.0f - pow(roughness, MISC::GET_FRAME_TIME());
}

void CFPVScript::updateLongitudinalCameraMovement(const CConfig::SMovement& movement) {
    float lerpF = getMovementLerpFactor(movement);

    float gForce = mVehicleData.Acceleration().y / 9.81f;

    //gForce = abs(pow(gForce, g_settings().Misc.Camera.Movement.LongGamma)) * sgn(gForce);

    float mappedAccel = 0.0f;
    float deadzone = movement.LongDeadzone;

    float mult = 0.0f;
    // Accelerate
    if (gForce > deadzone) {
        mappedAccel = map(gForce, deadzone, 10.0f, 0.0f, 10.0f);
        mult = movement.LongBackwardMult;
    }
    // Decelerate
    if (gForce < -deadzone) {
        mappedAccel = map(gForce, -deadzone, -10.0f, 0.0f, -10.0f);
        mult = movement.LongForwardMult;
    }
    float longBwLim = movement.LongBackwardLimit;
    float longFwLim = movement.LongForwardLimit;
    float accelVal =
        std::clamp(-mappedAccel * mult,
            -longBwLim,
            longFwLim);
    mInertiaMove.y = lerp(mInertiaMove.y, accelVal, lerpF); // just for smoothness
}

void CFPVScript::updateLateralCameraMovement(const CConfig::SMovement& movement) {
    float lerpF = getMovementLerpFactor(movement);

    auto accelVec = mVehicleData.AccelerationCentripetal();
    float gForce = accelVec.x / 9.8f;

    float mappedAccel = 0.0f;
    const float deadzone = movement.LatDeadzone;

    float mult = 0.0f;

    if (abs(gForce) > deadzone) {
        mappedAccel = map(gForce, deadzone, 10.0f, 0.0f, 10.0f);
        mult = movement.LatMult;
    }
    float latLim = movement.LatLimit;

    float accelVal =
        std::clamp(mappedAccel * mult,
            -latLim,
            latLim);
    mInertiaMove.x = lerp(mInertiaMove.x, accelVal, lerpF); // just for smoothness
}

void CFPVScript::updateVerticalCameraMovement(const CConfig::SMovement& movement) {
    float lerpF = getMovementLerpFactor(movement);

    auto accelVec = mVehicleData.AccelerationCentripetal();
    float gForce = accelVec.z / 9.8f;

    float mappedAccel = 0.0f;
    const float deadzone = movement.VertDeadzone;

    float mult = 0.0f;

    // Up
    if (gForce > deadzone) {
        mappedAccel = map(gForce, deadzone, 10.0f, 0.0f, 10.0f);
        mult = movement.VertDownMult;
    }

    // Down
    if (gForce < -deadzone) {
        mappedAccel = map(gForce, -deadzone, -10.0f, 0.0f, -10.0f);
        mult = movement.VertUpMult;
    }

    float accelVal =
        std::clamp(-mappedAccel * mult,
            -movement.VertDownLimit,
            movement.VertUpLimit);
    mInertiaMove.y = lerp(mInertiaMove.y, accelVal, lerpF); // just for smoothness
}

void CFPVScript::updatePitchCameraMovement(const CConfig::SMovement& movement) {
    float lerpF = getMovementLerpFactor(movement);

    float gForce = mVehicleData.AccelerationCentripetal().y / 9.81f;

    float mappedAccel = 0.0f;
    float deadzone = movement.PitchDeadzone;

    float mult = 0.0f;
    // Accelerate
    if (gForce > deadzone) {
        mappedAccel = map(gForce, deadzone, 10.0f, 0.0f, 10.0f);
        mult = movement.PitchUpMult;
    }
    // Decelerate
    if (gForce < -deadzone) {
        mappedAccel = map(gForce, -deadzone, -10.0f, 0.0f, -10.0f);
        mult = movement.PitchDownMult;
    }
    float pitchUpLim = movement.PitchUpMaxAngle;
    float pitchDownLim = movement.PitchDownMaxAngle;
    float pitchVal =
        std::clamp(mappedAccel * mult,
            -pitchDownLim, pitchUpLim);
    mInertiaPitch = lerp(mInertiaPitch, pitchVal, lerpF); // just for smoothness
}

void CFPVScript::updateDoF(const CConfig::SDoF& dof) {
    CAM::SET_USE_HI_DOF(); // Call each frame
    CAM::SET_CAM_USE_SHALLOW_DOF_MODE(mHandle, true); // Depends on SET_USE_HI_DOF, so also each frame?

    // smooth out defocusing/focusing
    auto lerpFactor = 1.0f - pow(0.01f, MISC::GET_FRAME_TIME());
    mAverageAccel = lerp(mAverageAccel, Length(mVehicleData.AccelerationCentripetal()), lerpFactor);

    float averageAcceleration =
        std::clamp(mAverageAccel, dof.TargetAccelMinDoF, dof.TargetAccelMaxDoF);

    const float vehMaxSpeed = VEHICLE::GET_VEHICLE_ESTIMATED_MAX_SPEED(mVehicle) / 0.75f;
    const float speed = ENTITY::GET_ENTITY_SPEED(mVehicle);
    const float speedRatio = speed / vehMaxSpeed;

    if (mSettings->Debug.Enable) {
        UI::ShowText(0.5f, 0.25f, 0.5f, std::format("Est Max Spd {:.0f} kph", vehMaxSpeed * 3.6f));
    }

    float nearDoF1 = mapclamp(speedRatio,
        dof.TargetSpeedMinDoF, dof.TargetSpeedMaxDoF,
        dof.NearOutFocusMinSpeedDist, dof.NearOutFocusMaxSpeedDist);

    nearDoF1 = mapclamp(averageAcceleration,
        dof.TargetAccelMinDoF, dof.TargetAccelMaxDoF,
        nearDoF1 * dof.TargetAccelMinDoFMod, nearDoF1 * dof.TargetAccelMaxDoFMod);

    float nearDoF2 = mapclamp(speedRatio,
        dof.TargetSpeedMinDoF, dof.TargetSpeedMaxDoF,
        dof.NearInFocusMinSpeedDist, dof.NearInFocusMaxSpeedDist);

    nearDoF2 = mapclamp(averageAcceleration,
        dof.TargetAccelMinDoF, dof.TargetAccelMaxDoF,
        nearDoF2 * dof.TargetAccelMinDoFMod, nearDoF2 * dof.TargetAccelMaxDoFMod);

    float farDoF1 = mapclamp(speedRatio,
        dof.TargetSpeedMinDoF, dof.TargetSpeedMaxDoF,
        dof.FarInFocusMinSpeedDist, dof.FarInFocusMaxSpeedDist);

    farDoF1 = mapclamp(averageAcceleration,
        dof.TargetAccelMinDoF, dof.TargetAccelMaxDoF,
        farDoF1 / dof.TargetAccelMinDoFMod, farDoF1 / dof.TargetAccelMaxDoFMod);

    float farDoF2 = mapclamp(speedRatio,
        dof.TargetSpeedMinDoF, dof.TargetSpeedMaxDoF,
        dof.FarOutFocusMinSpeedDist, dof.FarOutFocusMaxSpeedDist);

    farDoF2 = mapclamp(averageAcceleration,
        dof.TargetAccelMinDoF, dof.TargetAccelMaxDoF,
        farDoF2 / dof.TargetAccelMinDoFMod, farDoF2 / dof.TargetAccelMaxDoFMod);

    if (mSettings->Debug.DoF.Override) {
        nearDoF1 = mSettings->Debug.DoF.NearOutFocus;
        nearDoF2 = mSettings->Debug.DoF.NearInFocus;
        farDoF1 = mSettings->Debug.DoF.FarInFocus;
        farDoF2 = mSettings->Debug.DoF.FarOutFocus;
    }
    CAM::SET_CAM_DOF_PLANES(mHandle, nearDoF1, nearDoF2, farDoF1, farDoF2);
}

Vector3 CFPVScript::getLeanOffset(bool lookingIntoGlass) const {
    Vector3 leanOffset{};

    const auto& mount = mActiveConfig->Mount[mActiveConfig->CamIndex];

    float leanLimitGlass = lookingIntoGlass ? 0.0f : mount.Lean.CenterDist;

    // Left
    if (mRotation.z > 85.0f) {
        leanOffset.x = map(mRotation.z, 85.0f, 180.0f, 0.0f, -mount.Lean.CenterDist);
        leanOffset.x = std::clamp(leanOffset.x, -mount.Lean.CenterDist, leanLimitGlass);

        float frontLean = map(mRotation.z, 85.0f, 180.0f, 0.0f, mount.Lean.ForwardDist);
        frontLean = std::clamp(frontLean, 0.0f, mount.Lean.ForwardDist);
        leanOffset.y += frontLean;
    }
    // Right
    if (mRotation.z < -85.0f) {
        leanOffset.x = map(mRotation.z, -85.0f, -180.0f, 0.0f, mount.Lean.CenterDist);
        leanOffset.x = std::clamp(leanOffset.x, -leanLimitGlass, mount.Lean.CenterDist);

        float frontLean = map(mRotation.z, -85.0f, -180.0f, 0.0f, mount.Lean.ForwardDist);
        frontLean = std::clamp(frontLean, 0.0f, mount.Lean.ForwardDist);
        leanOffset.y += frontLean;
    }
    // Don't care
    if (!lookingIntoGlass && abs(mRotation.z) > 85.0f) {
        float upPeek = map(abs(mRotation.z), 85.0f, 160.0f, 0.0f, mount.Lean.UpDist);
        upPeek = std::clamp(upPeek, 0.0f, mount.Lean.UpDist);
        leanOffset.z += upPeek;
    }

    return leanOffset;
}

Vector3 CFPVScript::getHorizonLockRotation() {
    const auto& mount = mActiveConfig->Mount[mActiveConfig->CamIndex];

    bool horLock = mount.HorizonLock.Lock;
    if (!horLock)
        return {};

    Vector3 rotations{};
    const float horPitchLim = mount.HorizonLock.PitchLim;
    const float horRollLim = mount.HorizonLock.RollLim;

    auto vehRot = ENTITY::GET_ENTITY_ROTATION(mVehicle, 0);
    auto vehPitch = ENTITY::GET_ENTITY_PITCH(mVehicle);
    auto vehRoll = ENTITY::GET_ENTITY_ROLL(mVehicle);
    float dynamicPitch = 0.0f;

    if (abs(vehPitch) > 90.0f) {
        vehPitch = map(abs(vehPitch), 90.0f, 180.0f, 90.0f, 0.0f) * sgn(vehPitch);
    }
    else {
        vehPitch = std::clamp(vehPitch, -horPitchLim, horPitchLim);
    }

    if (abs(vehRoll) > 90.0f) {
        vehRoll = map(abs(vehRoll), 90.0f, 180.0f, 90.0f, 0.0f) * sgn(vehRoll);
    }
    else {
        vehRoll = std::clamp(vehRoll, -horRollLim, horRollLim);
    }

    switch (mount.HorizonLock.PitchMode) {
        case 2:
        {
            float rate = MISC::GET_FRAME_TIME() * mount.HorizonLock.CenterSpeed;
            mDynamicPitch = rate * (vehPitch)+(1.0f - rate) * mDynamicPitch;
            dynamicPitch = vehPitch - mDynamicPitch;
            dynamicPitch = std::clamp(dynamicPitch, -horPitchLim, horPitchLim);
            mDynamicPitch = std::clamp(mDynamicPitch, -horPitchLim, horPitchLim);
            break;
        }
        case 1:
        {
            dynamicPitch = 0.0f;
            break;
        }
        case 0: [[fallthrough]];
        default:
        {
            dynamicPitch = vehPitch;
        }
    }

    rotations.x = abs(mRotation.z) <= 90.0f ?
        map(abs(mRotation.z), 0.0f, 90.0f, dynamicPitch, 0.0f) :
        map(abs(mRotation.z), 90.0f, 180.0f, 0.0f, -dynamicPitch + vehRot.x * 2.0f * abs(mRotation.z) / 180.0f);

    rotations.y = abs(mRotation.z) <= 90.0f ?
        map(abs(mRotation.z), 0.0f, 90.0f, vehRoll, 0.0f) :
        map(abs(mRotation.z), 90.0f, 180.0f, 0.0f, vehRoll);

    return rotations;
}

Vector3 CFPVScript::getShakeFromSpeed() {
    if (!VEHICLE::IS_VEHICLE_ON_ALL_WHEELS(mVehicle))
        return {};

    const float amplitudeBase = mActiveConfig->Mount[mActiveConfig->CamIndex].Movement.ShakeSpeed;

    const double sideZ = 3.3f;
    const double vertZ = 4.2f;
    const double rollZ = 6.9f;

    const float minRateMod = mShakeData->MinRateModSpd;
    const float maxRateMod = mShakeData->MaxRateModSpd;

    const float vehMaxSpeed = VEHICLE::GET_VEHICLE_ESTIMATED_MAX_SPEED(mVehicle) / 0.75f;
    const float speed = ENTITY::GET_ENTITY_SPEED(mVehicle);

    if (mSettings->Debug.Enable) {
        UI::ShowText(0.5f, 0.25f, 0.5f, std::format("Est Max Spd {:.0f} kph", vehMaxSpeed * 3.6f));
    }

    // Shake amplitude modifer <-> rpm
    float rpmModifier = mapclamp(VExt::GetRPM(mVehicle), 0.5f, 0.8f, 0.0f, 1.0f);

    // Shake amplitude <-> speed relation:
    // <  50% speed: no shake
    // >  90% speed: full shake
    float amplitude = mapclamp(speed,
        vehMaxSpeed * 0.4f,
        vehMaxSpeed * 0.7f,
        0.0f,
        amplitudeBase * rpmModifier);

    double x = cos(mCumTimeSpeed);
    double y = sin(mCumTimeSpeed);

    double sideNoise = mPerlinNoise->noise(x, y, sideZ + mCumTimeSpeed) - 0.5;
    double vertNoise = mPerlinNoise->noise(x, y, vertZ + mCumTimeSpeed) - 0.5;
    double rollNoise = mPerlinNoise->noise(x, y, rollZ + mCumTimeSpeed) - 0.5;

    float shakeRate = mapclamp(speed, 0.0f, vehMaxSpeed, minRateMod, maxRateMod);
    mCumTimeSpeed = mCumTimeSpeed + MISC::GET_FRAME_TIME() * Memory::GetTimeScale() * shakeRate;

    return Vector3{
        static_cast<float>(sideNoise * amplitude),
        static_cast<float>(vertNoise * amplitude),
        static_cast<float>(rollNoise * 5.0 * amplitude)
    };
}

Vector3 CFPVScript::getShakeFromTerrain() {
    const float amplitudeBase = mActiveConfig->Mount[mActiveConfig->CamIndex].Movement.ShakeTerrain;

    const double sideZ = 3.3f;
    const double vertZ = 4.2f;
    const double rollZ = 6.9f;

    const float minRateMod = mShakeData->MinRateModTrn;
    const float maxRateMod = mShakeData->MaxRateModTrn;

    const float vehMaxSpeed = VEHICLE::GET_VEHICLE_ESTIMATED_MAX_SPEED(mVehicle) / 0.75f;
    const float speed = ENTITY::GET_ENTITY_SPEED(mVehicle);

    auto terrainTypes = VExt::GetTireContactMaterials(mVehicle);
    auto suspensionCompressions = VExt::GetSuspensionCompressions(mVehicle);

    float terrainAmplMod = 0.0f;
    float terrainFreqMod = 1.0f;
    float terrainMatchCount = 0.0f;
    for (int i = 0; i < terrainTypes.size(); ++i) {
        auto terrainType = terrainTypes[i];
        auto foundMatIt = mShakeData->MaterialReactionMap.find(static_cast<eMaterial>(terrainType));
        bool onGround = suspensionCompressions[i] > 0.0f;

        if (onGround && foundMatIt != mShakeData->MaterialReactionMap.end()) {
            terrainAmplMod += foundMatIt->second.Amplitude / static_cast<float>(terrainTypes.size());

            terrainFreqMod += foundMatIt->second.Frequency;
            terrainMatchCount += 1.0f;
        }

        if (mSettings->Debug.Enable) {
            std::string matName = "Unknown";
            std::string matParams = "None";
            if (terrainType < sMaterialNames.size()) {
                matName = sMaterialNames[terrainType];
            }

            if (foundMatIt != mShakeData->MaterialReactionMap.end()) {
                matParams = std::format("[A {:.2f} | F {:.2f}]",
                    foundMatIt->second.Amplitude, foundMatIt->second.Frequency);
            }

            UI::ShowText(0.25f, 0.05f * i, 0.5f, std::format("[{}] {} @ {}{}",
                i, matName, onGround ? "~g~" : "", matParams));
        }
    }

    if (terrainMatchCount > 0.0f) {
        terrainFreqMod /= terrainMatchCount;
    }

    float amplitude = mapclamp(speed,
        0.0f,
        vehMaxSpeed * 0.3f,
        0.0f,
        amplitudeBase * terrainAmplMod);

    double x = cos(mCumTimeTerrain);
    double y = sin(mCumTimeTerrain);

    double sideNoise = mPerlinNoise->noise(x, y, sideZ + mCumTimeTerrain) - 0.5;
    double vertNoise = mPerlinNoise->noise(x, y, vertZ + mCumTimeTerrain) - 0.5;
    double rollNoise = mPerlinNoise->noise(x, y, rollZ + mCumTimeTerrain) - 0.5;

    float shakeRate = mapclamp(speed, 0.0f, vehMaxSpeed, minRateMod, maxRateMod) * terrainFreqMod;
    mCumTimeTerrain = mCumTimeTerrain + MISC::GET_FRAME_TIME() * Memory::GetTimeScale() * shakeRate;

    return Vector3{
        static_cast<float>(sideNoise * amplitude),
        static_cast<float>(vertNoise * amplitude),
        static_cast<float>(rollNoise * 5.0 * amplitude)
    };
}
