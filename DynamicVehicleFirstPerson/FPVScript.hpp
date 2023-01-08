#pragma once
#include "Compatibility.hpp"
#include "Config.hpp"
#include "ScriptSettings.hpp"
#include "VehicleMetaData.hpp"
#include "Util/Timer.hpp"

#include <inc/types.h>
#include <memory>
#include <string>

class CFPVScript {
public:
    CFPVScript(const std::shared_ptr<CScriptSettings>& settings, std::vector<CConfig>& configs);
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

    void updateControllerLook(bool& lookingIntoGlass);
    void updateMouseLook(bool& lookingIntoGlass);
    void updateWheelLook(bool& lookingIntoGlass);

    void updateRotationCameraMovement(const CConfig::SMovement& movement);
    void updateLongitudinalCameraMovement(const CConfig::SMovement& movement);
    void updateLateralCameraMovement(const CConfig::SMovement& movement);
    void updateVerticalCameraMovement(const CConfig::SMovement& movement);
    void updatePitchCameraMovement(const CConfig::SMovement& movement);
    void updateDoF(const CConfig::SDoF& dof);

    Vector3 getLeanOffset(bool lookingIntoGlass) const;
    Vector3 getHorizonLockRotation();

    // Config management
    const std::shared_ptr<CScriptSettings>& mSettings;
    std::vector<CConfig>& mConfigs;
    CConfig& mDefaultConfig;
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
};
