#pragma once
#include <inc/types.h>
#include <string>
#include <vector>

class CConfig {
public:
    enum class ESaveType {
        Specific,       // [ID] writes Model + Plate
        GenericModel,   // [ID] writes Model
        GenericNone,    // [ID] writes none
    };

    struct SLean {
        float CenterDist = 0.25f;
        float ForwardDist = 0.05f;
        float UpDist = 0.08f;
    };

    struct SMovement {
        bool Follow = true;
        float RotationDirectionMult = 0.50f;
        float RotationRotationMult = 0.05f;
        float RotationMaxAngle = 45.0f;

        float LongDeadzone = 0.0f;
        float LongForwardMult = 0.05f;
        float LongBackwardMult = 0.05f;
        float LongForwardLimit = 0.07f;
        float LongBackwardLimit = 0.07f;

        float PitchDeadzone = 0.0f;
        float PitchUpMult = 1.0f;
        float PitchDownMult = 1.0f;
        float PitchUpMaxAngle = 5.0f;
        float PitchDownMaxAngle = 5.0f;

        float LatDeadzone = 0.0f;
        float LatMult = 0.04f;
        float LatLimit = 0.02f;

        float VertDeadzone = 0.0f;
        float VertUpMult = 0.05f;
        float VertDownMult = 0.10f;
        float VertUpLimit = 0.05f;
        float VertDownLimit = 0.06f;

        float Roughness = 1.0f;
    };

    struct SHorizonLock {
        bool Lock = false;
        // 0: Lock camera pitch with horizon
        // 1: Lock camera pitch with car
        // 2: Dynamic camera pitch
        int PitchMode = 0;
        // 0.01: Quick
        // 1.00: Slow
        float CenterSpeed = 1.5f;
        float PitchLim = 30.0f;
        float RollLim = 45.0f;
    };

    struct SDoF {
        bool Enable = false;

        // Target speeds for DoF setpoints
        float TargetSpeedMinDoF = 36.0f; // ~130 kph
        float TargetSpeedMaxDoF = 72.0f; // ~260 kph

        // Target acceleration for DoF setpoint modifiers
        float TargetAccelMinDoF = 5.0f; // 0.5 G
        float TargetAccelMaxDoF = 9.8f; // 1.0 G

        float TargetAccelMinDoFMod = 0.1f; // accel <= targetAccelMinDoF: Move near plane closer, far plane farther
        float TargetAccelMaxDoFMod = 1.0f; // accel >= targetAccelMaxDoF: Move near plane farther, far plane closer

        // Near DoF: Close plane, max blur
        float NearOutFocusMinSpeedDist = 0.0f;
        float NearOutFocusMaxSpeedDist = 0.5f; // 50 cm for blurring dashboard

        // Near DoF: Far plane, min blur
        // Should be farther than close plane
        float NearInFocusMinSpeedDist = 0.1f;
        float NearInFocusMaxSpeedDist = 20.0f; // 20m for blurring front of car and road

        // Far DoF: Close plane, min blur
        float FarInFocusMinSpeedDist = 100000.0f; // Infinite focus (100km)
        float FarInFocusMaxSpeedDist = 2000.0f;  // Start blurring beyond 2km

        // Far DoF: Far plane, max blur
        float FarOutFocusMinSpeedDist = 100000.0f; // Infinite focus
        float FarOutFocusMaxSpeedDist = 10000.0f; // Max blur > 10km
    };

    enum class EMountPoint {
        Vehicle,
        Ped
    };

    struct SCameraSettings {
        EMountPoint MountPoint = EMountPoint::Vehicle;

        float FOV = 55.0f;
        float OffsetHeight = 0.04f;
        float OffsetForward = 0.05f;
        float OffsetSide = 0.0f;
        float Pitch = 0.0f;
        SLean Lean;
        SHorizonLock HorizonLock;
        SMovement Movement;
        SDoF DoF;
    };

    CConfig() = default;
    static CConfig Read(const std::string& configFile);

    void Write(ESaveType saveType);
    bool Write(const std::string& newName, Hash model, std::string plate, ESaveType saveType);

    std::string Name;

    // ID
    Hash ModelHash = 0;
    std::string ModelName;
    std::string Plate;

    // Main
    bool Enable = true;
    int CamIndex = 0;

    // Look
    struct {
        float LookTime = 0.000010f;
        float MouseLookTime = 0.000001f;
        int MouseCenterTimeout = 750;
        float MouseSensitivity = 0.3f;
    } Look;

    // [Mount0-9]
    std::vector<SCameraSettings> Mount;
};
