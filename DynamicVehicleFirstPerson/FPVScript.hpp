#pragma once
#include "Compatibility.hpp"
#include "Config.hpp"
#include "ScriptSettings.hpp"
#include "ShakeData.hpp"
#include "VehicleMetaData.hpp"
#include "Util/Timer.hpp"

#include <inc/types.h>
#include <PerlinNoise.h>
#include <memory>
#include <string>

class CFPVScript {
public:
    CFPVScript(const std::shared_ptr<CScriptSettings>& settings,
        const std::shared_ptr<CShakeData>& shakeData,
        std::vector<CConfig>& configs);
    ~CFPVScript() = default;

    void UpdateActiveConfig();
    CConfig* ActiveConfig() {
        return mActiveConfig;
    }

    Vehicle GetVehicle() {
        return mVehicle;
    }

    void Tick();
    void Cancel();

    void HideHead(bool remove) { hideHead(remove); }
private:
    void update();
    
    void init();
    void hideHead(bool remove);

    float getRearLookAngle(ESeatPosition seatPosition, float lookLeftRight, float maxAngle);
    void updateControllerLook(bool& lookingIntoGlass);
    void updateMouseLook(bool& lookingIntoGlass);
    void updateWheelLook(bool& lookingIntoGlass);

    void updateRotationCameraMovement(const CConfig::SMovement& movement);

    float getMovementLerpFactor(const CConfig::SMovement& movement) const;
    void updateLongitudinalCameraMovement(const CConfig::SMovement& movement);
    void updateLateralCameraMovement(const CConfig::SMovement& movement);
    void updateVerticalCameraMovement(const CConfig::SMovement& movement);
    void updatePitchCameraMovement(const CConfig::SMovement& movement);

    void updateDoF(const CConfig::SDoF& dof);

    Vector3 getLeanOffset(bool lookingIntoGlass) const;
    Vector3 getHorizonLockRotation();

    // X, Z, Roll
    Vector3 getShakeFromSpeed();
    Vector3 getShakeFromTerrain();

    // Config management
    const std::shared_ptr<CScriptSettings>& mSettings;
    const std::shared_ptr<CShakeData>& mShakeData;
    std::vector<CConfig>& mConfigs;
    CConfig* mActiveConfig = nullptr;

    // Correct offset for <1290 on load
    unsigned mFpvCamOffsetXOffset = 0x450;

    Vehicle mVehicle;
    // Just create a new one each time mVehicle changes
    CVehicleMetaData mVehicleData;

    Cam mHandle = -1;

    Vector3 mRotation{};

    CSysTimer mLookResetTimer;

    // Accumulated values for mouse look
    Vector3 mLookAcc{};

    // rotation camera movement
    float mInertiaDirectionLookAngle = 0.0f;

    // forward camera movement
    Vector3 mInertiaMove{};

    // in degrees
    float mInertiaPitch = 0.0f;

    // in degrees
    float mDynamicPitch = 0.0f;

    bool mHeadRemoved = false;
    int savedHeadProp = -1;
    int savedHeadPropTx = -1;
    int savedEyesProp = -1;
    int savedEyesPropTx = -1;

    float mAverageAccel = 0.0f;

    // For Manual Transmission
    // Figure out which side to look "back" with:
    // When the LookRight is pressed first, and then LookLeft is pressed
    // camera should look back over the right shoulder - otherwise left shoulder.
    bool mMTLookRightPrev = false;
    bool mMTLookLeftPrev = false;
    bool mMTLookBackRightShoulder = false;

    std::unique_ptr<PerlinNoise> mPerlinNoise;
    double mCumTimeSpeed = 0.0;
    double mCumTimeTerrain = 0.0;
};
