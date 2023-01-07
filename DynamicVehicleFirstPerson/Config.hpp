#pragma once
#include <string>
#include <vector>

template <typename T>
class Tracked {
    T mValue;
    T mInitialValue;
    bool mChanged = false;
public:
    Tracked(T val) : mValue(val), mInitialValue(val), mChanged(false) {}
    Tracked& operator=(T v) { mChanged = true; mValue = v; return *this; }

    T Value() const { return mValue; }

    operator T() const { return mValue; }
    operator T& () { if (mValue != mInitialValue) { mChanged = true; } return mValue; }

    bool operator==(const T& rhs) { return mValue == rhs; }
    bool operator!=(const T& rhs) { return !(mValue == rhs); }

    bool Changed() const { return mChanged || mValue != mInitialValue; }
    void Set(T val) { mValue = mInitialValue = val; mChanged = false; }
    void Reset() { mInitialValue = mValue; mChanged = false; }
};

class CConfig {
public:
    CConfig();
    void SetFiles(CConfig* baseConfig, const std::string& file);

    void LoadSettings();
    void SaveSettings();
    void SaveSettings(CConfig* baseConfig, const std::string& customPath);

    struct SMovement {
        Tracked<bool> Follow = true;
        Tracked<float> RotationDirectionMult = 0.50f;
        Tracked<float> RotationRotationMult = 0.05f;
        Tracked<float> RotationMaxAngle = 45.0f;

        Tracked<float> LongDeadzone = 0.0f;
        Tracked<float> LongForwardMult = 0.05f;
        Tracked<float> LongBackwardMult = 0.05f;
        Tracked<float> LongForwardLimit = 0.07f;
        Tracked<float> LongBackwardLimit = 0.07f;

        Tracked<float> PitchDeadzone = 0.0f;
        Tracked<float> PitchUpMult = 1.0f;
        Tracked<float> PitchDownMult = 1.0f;
        Tracked<float> PitchUpMaxAngle = 5.0f;
        Tracked<float> PitchDownMaxAngle = 5.0f;

        Tracked<float> LatDeadzone = 0.0f;
        Tracked<float> LatMult = 0.04f;
        Tracked<float> LatLimit = 0.02f;

        Tracked<float> VertDeadzone = 0.0f;
        Tracked<float> VertUpMult = 0.05f;
        Tracked<float> VertDownMult = 0.10f;
        Tracked<float> VertUpLimit = 0.05f;
        Tracked<float> VertDownLimit = 0.06f;

        Tracked<float> Roughness = 1.0f;
    };

    struct SHorizonLock {
        Tracked<bool> Lock = false;
        // 0: Lock camera pitch with horizon
        // 1: Lock camera pitch with car
        // 2: Dynamic camera pitch
        Tracked<int> PitchMode = 0;
        // 0.01: Quick
        // 1.00: Slow
        Tracked<float> CenterSpeed = 1.5f;
        Tracked<float> PitchLim = 30.0f;
        Tracked<float> RollLim = 45.0f;
    };

    struct SDoF {
        Tracked<bool> Enable = false;

        // Target speeds for DoF setpoints
        Tracked<float> TargetSpeedMinDoF = 36.0f; // ~130 kph
        Tracked<float> TargetSpeedMaxDoF = 72.0f; // ~260 kph

        // Target acceleration for DoF setpoint modifiers
        Tracked<float> TargetAccelMinDoF = 5.0f; // 0.5 G
        Tracked<float> TargetAccelMaxDoF = 9.8f; // 1.0 G

        Tracked<float> TargetAccelMinDoFMod = 0.1f; // accel <= targetAccelMinDoF: Move near plane closer, far plane farther
        Tracked<float> TargetAccelMaxDoFMod = 1.0f; // accel >= targetAccelMaxDoF: Move near plane farther, far plane closer

        // Near DoF: Close plane, max blur
        Tracked<float> NearOutFocusMinSpeedDist = 0.0f;
        Tracked<float> NearOutFocusMaxSpeedDist = 0.5f; // 50 cm for blurring dashboard

        // Near DoF: Far plane, min blur
        // Should be farther than close plane
        Tracked<float> NearInFocusMinSpeedDist = 0.1f;
        Tracked<float> NearInFocusMaxSpeedDist = 20.0f; // 20m for blurring front of car and road

        // Far DoF: Close plane, min blur
        Tracked<float> FarInFocusMinSpeedDist = 100000.0f; // Infinite focus (100km)
        Tracked<float> FarInFocusMaxSpeedDist = 2000.0f;  // Start blurring beyond 2km

        // Far DoF: Far plane, max blur
        Tracked<float> FarOutFocusMinSpeedDist = 100000.0f; // Infinite focus
        Tracked<float> FarOutFocusMaxSpeedDist = 10000.0f; // Max blur > 10km
    };

    struct SCameraSettings {
        Tracked<float> FOV = 55.0f;
        Tracked<float> OffsetHeight = 0.04f;
        Tracked<float> OffsetForward = 0.05f;
        Tracked<float> OffsetSide = 0.0f;
        Tracked<float> Pitch = 0.0f;
        SHorizonLock HorizonLock;
        SMovement Movement;
        SDoF DoF;
    };

    enum class EMount {
        Vehicle,
        Ped
    };

    std::string Name;

    // ID
    std::vector<std::string> ModelNames;
    std::vector<std::string> Plates;
    std::string Description;

    // Main
    Tracked<bool> Enable = true;

    // Always one Ped/Vehicle, max [9]
    Tracked<EMount> MountPoint = EMount::Vehicle;
    Tracked<int> MountIdPed = 1;
    Tracked<int> MountIdVehicle = 1;

    // Look
    struct {
        Tracked<float> LookTime = 0.000010f;
        Tracked<float> MouseLookTime = 0.000001f;
        Tracked<int> MouseCenterTimeout = 750;
        Tracked<float> MouseSensitivity = 0.3f;
    } Look;

    // [Vehicle0-9]
    std::vector<SCameraSettings> Vehicle;

    // [Ped0-9]
    std::vector<SCameraSettings> Ped;
private:
    void saveGeneral();

    std::string mFile;

    // Reference to one unique "master" instance.
    CConfig* mBaseConfig;
};
