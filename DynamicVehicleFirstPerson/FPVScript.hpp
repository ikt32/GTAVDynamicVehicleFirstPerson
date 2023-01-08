#pragma once
#include "Compatibility.hpp"
#include "Config.hpp"
#include "ScriptSettings.hpp"
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
private:
    void update();
    
    void hideHead(bool remove);

    // Config management
    const std::shared_ptr<CScriptSettings>& mSettings;
    std::vector<CConfig>& mConfigs;
    CConfig& mDefaultConfig;
    CConfig* mActiveConfig = nullptr;

    // Correct offset for <1290 on load
    unsigned mFpvCamOffsetXOffset = 0x450;

    Vehicle mVehicle;

    Cam mHandle = -1;

    Vector3 mRotation{};

    CSysTimer mLookResetTimer;

    // Accumulated values for mouse look
    Vector3 mLookAcc{};

    // rotation camera movement
    float mInertiaDirectionLookAngle = 0.0f;

    // forward camera movement
    Vector3 mInertiaMove{};
    float mInertiaAccelPitchDeg = 0.0f;

    float mDynamicPitchDeg = 0.0f;

    bool mHeadRemoved = false;
    int savedHeadProp = -1;
    int savedHeadPropTx = -1;
    int savedEyesProp = -1;
    int savedEyesPropTx = -1;

    float mAverageAccel = 0.0f;
};
