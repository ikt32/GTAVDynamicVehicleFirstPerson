#include "Config.hpp"

#include "Constants.hpp"
#include "SettingsCommon.hpp"
#include "Util/Logger.hpp"
#include "Util/Strings.hpp"

#include <simpleini/SimpleIni.h>
#include <filesystem>
#include <cctype>

#define CHECK_LOG_SI_ERROR(result, operation) \
    if (result < 0) { \
        LOG(ERROR, "[CConfig] {} Failed to {}, SI_Error [{}]", \
        __FUNCTION__, operation, result); \
    }

#define SAVE_VAL(section, key, option) \
    if (mBaseConfig == this || option != mBaseConfig->option || option.Changed()) { \
        SetValue(ini, section, key, option); \
        option.Reset(); \
    }

#define LOAD_VAL(section, key, option) \
    option.Set(GetValue(ini, section, key, baseConfig.option))

#define SAVE_VAL_MOVEMENT(section, source) { \
    SAVE_VAL(section, "FollowMovement",    ##source.Follow); \
    SAVE_VAL(section, "MovementMultVel",   ##source.RotationDirectionMult); \
    SAVE_VAL(section, "MovementMultRot",   ##source.RotationRotationMult); \
    SAVE_VAL(section, "MovementCap",       ##source.RotationMaxAngle); \
    SAVE_VAL(section, "LongDeadzone",      ##source.LongDeadzone); \
    SAVE_VAL(section, "LongForwardMult",   ##source.LongForwardMult); \
    SAVE_VAL(section, "LongBackwardMult",  ##source.LongBackwardMult); \
    SAVE_VAL(section, "LongForwardLimit",  ##source.LongForwardLimit); \
    SAVE_VAL(section, "LongBackwardLimit", ##source.LongBackwardLimit); \
    SAVE_VAL(section, "PitchDeadzone",     ##source.PitchDeadzone); \
    SAVE_VAL(section, "PitchUpMult",       ##source.PitchUpMult); \
    SAVE_VAL(section, "PitchDownMult",     ##source.PitchDownMult); \
    SAVE_VAL(section, "PitchUpMaxAngle",   ##source.PitchUpMaxAngle); \
    SAVE_VAL(section, "PitchDownMaxAngle", ##source.PitchDownMaxAngle); \
    SAVE_VAL(section, "LatDeadzone",       ##source.LatDeadzone); \
    SAVE_VAL(section, "LatMult",           ##source.LatMult); \
    SAVE_VAL(section, "LatLimit",          ##source.LatLimit); \
    SAVE_VAL(section, "VertDeadzone",      ##source.VertDeadzone); \
    SAVE_VAL(section, "VertUpMult",        ##source.VertUpMult); \
    SAVE_VAL(section, "VertDownMult",      ##source.VertDownMult); \
    SAVE_VAL(section, "VertUpLimit",       ##source.VertUpLimit); \
    SAVE_VAL(section, "VertDownLimit",     ##source.VertDownLimit); \
    SAVE_VAL(section, "Roughness",         ##source.Roughness); \
}

#define LOAD_VAL_MOVEMENT(section, source) { \
    LOAD_VAL(section, "FollowMovement",    ##source.Follow); \
    LOAD_VAL(section, "MovementMultVel",   ##source.RotationDirectionMult); \
    LOAD_VAL(section, "MovementMultRot",   ##source.RotationRotationMult); \
    LOAD_VAL(section, "MovementCap",       ##source.RotationMaxAngle); \
    LOAD_VAL(section, "LongDeadzone",      ##source.LongDeadzone); \
    LOAD_VAL(section, "LongForwardMult",   ##source.LongForwardMult); \
    LOAD_VAL(section, "LongBackwardMult",  ##source.LongBackwardMult); \
    LOAD_VAL(section, "LongForwardLimit",  ##source.LongForwardLimit); \
    LOAD_VAL(section, "LongBackwardLimit", ##source.LongBackwardLimit); \
    LOAD_VAL(section, "PitchDeadzone",     ##source.PitchDeadzone); \
    LOAD_VAL(section, "PitchUpMult",       ##source.PitchUpMult); \
    LOAD_VAL(section, "PitchDownMult",     ##source.PitchDownMult); \
    LOAD_VAL(section, "PitchUpMaxAngle",   ##source.PitchUpMaxAngle); \
    LOAD_VAL(section, "PitchDownMaxAngle", ##source.PitchDownMaxAngle); \
    LOAD_VAL(section, "LatDeadzone",       ##source.LatDeadzone); \
    LOAD_VAL(section, "LatMult",           ##source.LatMult); \
    LOAD_VAL(section, "LatLimit",          ##source.LatLimit); \
    LOAD_VAL(section, "VertDeadzone",      ##source.VertDeadzone); \
    LOAD_VAL(section, "VertUpMult",        ##source.VertUpMult); \
    LOAD_VAL(section, "VertDownMult",      ##source.VertDownMult); \
    LOAD_VAL(section, "VertUpLimit",       ##source.VertUpLimit); \
    LOAD_VAL(section, "VertDownLimit",     ##source.VertDownLimit); \
    LOAD_VAL(section, "Roughness",         ##source.Roughness); \
}

#define SAVE_VAL_HORIZON(section, source) { \
    SAVE_VAL(section, "HorLock",        ##source.Lock); \
    SAVE_VAL(section, "HorPitchMode",   ##source.PitchMode); \
    SAVE_VAL(section, "HorCenterSpeed", ##source.CenterSpeed); \
    SAVE_VAL(section, "HorPitchLim",    ##source.PitchLim); \
    SAVE_VAL(section, "HorRollLim",     ##source.RollLim); \
}

#define LOAD_VAL_HORIZON(section, source) { \
    LOAD_VAL(section, "HorLock",        ##source.Lock); \
    LOAD_VAL(section, "HorPitchMode",   ##source.PitchMode); \
    LOAD_VAL(section, "HorCenterSpeed", ##source.CenterSpeed); \
    LOAD_VAL(section, "HorPitchLim",    ##source.PitchLim); \
    LOAD_VAL(section, "HorRollLim",     ##source.RollLim); \
}

#define SAVE_VAL_DOF(section, source) { \
    SAVE_VAL(section, "DoFEnable",                   ##source.Enable); \
    SAVE_VAL(section, "DoFTargetSpeedMinDoF",        ##source.TargetSpeedMinDoF); \
    SAVE_VAL(section, "DoFTargetSpeedMaxDoF",        ##source.TargetSpeedMaxDoF); \
    SAVE_VAL(section, "DoFTargetAccelMinDoF",        ##source.TargetAccelMinDoF); \
    SAVE_VAL(section, "DoFTargetAccelMaxDoF",        ##source.TargetAccelMaxDoF); \
    SAVE_VAL(section, "DoFTargetAccelMinDoFMod",     ##source.TargetAccelMinDoFMod); \
    SAVE_VAL(section, "DoFTargetAccelMaxDoFMod",     ##source.TargetAccelMaxDoFMod); \
    SAVE_VAL(section, "DoFNearOutFocusMinSpeedDist", ##source.NearOutFocusMinSpeedDist); \
    SAVE_VAL(section, "DoFNearOutFocusMaxSpeedDist", ##source.NearOutFocusMaxSpeedDist); \
    SAVE_VAL(section, "DoFNearInFocusMinSpeedDist",  ##source.NearInFocusMinSpeedDist); \
    SAVE_VAL(section, "DoFNearInFocusMaxSpeedDist",  ##source.NearInFocusMaxSpeedDist); \
    SAVE_VAL(section, "DoFFarInFocusMinSpeedDist",   ##source.FarInFocusMinSpeedDist); \
    SAVE_VAL(section, "DoFFarInFocusMaxSpeedDist",   ##source.FarInFocusMaxSpeedDist); \
    SAVE_VAL(section, "DoFFarOutFocusMinSpeedDist",  ##source.FarOutFocusMinSpeedDist); \
    SAVE_VAL(section, "DoFFarOutFocusMaxSpeedDist",  ##source.FarOutFocusMaxSpeedDist); \
}

#define LOAD_VAL_DOF(section, source) { \
    LOAD_VAL(section, "DoFEnable",                   ##source.Enable); \
    LOAD_VAL(section, "DoFTargetSpeedMinDoF",        ##source.TargetSpeedMinDoF); \
    LOAD_VAL(section, "DoFTargetSpeedMaxDoF",        ##source.TargetSpeedMaxDoF); \
    LOAD_VAL(section, "DoFTargetAccelMinDoF",        ##source.TargetAccelMinDoF); \
    LOAD_VAL(section, "DoFTargetAccelMaxDoF",        ##source.TargetAccelMaxDoF); \
    LOAD_VAL(section, "DoFTargetAccelMinDoFMod",     ##source.TargetAccelMinDoFMod); \
    LOAD_VAL(section, "DoFTargetAccelMaxDoFMod",     ##source.TargetAccelMaxDoFMod); \
    LOAD_VAL(section, "DoFNearOutFocusMinSpeedDist", ##source.NearOutFocusMinSpeedDist); \
    LOAD_VAL(section, "DoFNearOutFocusMaxSpeedDist", ##source.NearOutFocusMaxSpeedDist); \
    LOAD_VAL(section, "DoFNearInFocusMinSpeedDist",  ##source.NearInFocusMinSpeedDist); \
    LOAD_VAL(section, "DoFNearInFocusMaxSpeedDist",  ##source.NearInFocusMaxSpeedDist); \
    LOAD_VAL(section, "DoFFarInFocusMinSpeedDist",   ##source.FarInFocusMinSpeedDist); \
    LOAD_VAL(section, "DoFFarInFocusMaxSpeedDist",   ##source.FarInFocusMaxSpeedDist); \
    LOAD_VAL(section, "DoFFarOutFocusMinSpeedDist",  ##source.FarOutFocusMinSpeedDist); \
    LOAD_VAL(section, "DoFFarOutFocusMaxSpeedDist",  ##source.FarOutFocusMaxSpeedDist); \
}

#define SAVE_VAL_CAMERA(section, source) { \
    SAVE_VAL(section, "FOV",           ##source.FOV); \
    SAVE_VAL(section, "OffsetHeight",  ##source.OffsetHeight); \
    SAVE_VAL(section, "OffsetForward", ##source.OffsetForward); \
    SAVE_VAL(section, "OffsetSide",    ##source.OffsetSide); \
    SAVE_VAL(section, "Pitch",         ##source.Pitch); \
}

#define LOAD_VAL_CAMERA(section, source) { \
    LOAD_VAL(section, "FOV",           ##source.FOV); \
    LOAD_VAL(section, "OffsetHeight",  ##source.OffsetHeight); \
    LOAD_VAL(section, "OffsetForward", ##source.OffsetForward); \
    LOAD_VAL(section, "OffsetSide",    ##source.OffsetSide); \
    LOAD_VAL(section, "Pitch",         ##source.Pitch); \
}

CConfig::CConfig() : mBaseConfig(nullptr) {}

void CConfig::SetFiles(CConfig* baseConfig, const std::string& file) {
    mBaseConfig = baseConfig;
    mFile = file;
}

#pragma warning(push)
#pragma warning(disable: 4244)
void CConfig::LoadSettings() {
    CConfig* pConfig = mBaseConfig;

    // Current instance is the base config.
    if (!mBaseConfig) {
        pConfig = this;
    }

    auto& baseConfig = *pConfig;

    CSimpleIniA ini;
    ini.SetUnicode();
    SI_Error result = ini.LoadFile(mFile.c_str());
    CHECK_LOG_SI_ERROR(result, std::format("load {}", mFile));

    Name = std::filesystem::path(mFile).stem().string();

    // [ID]
    std::string allNames = ini.GetValue("ID", "ModelName", "");
    std::string allPlates = ini.GetValue("ID", "Plate", "");
    ModelNames = StrUtil::Split(allNames, ' ');

    Plates = StrUtil::Split(allPlates, ',');
    for (auto& plate : Plates) {
        auto first = plate.find_first_of('[');
        auto last = plate.find_last_of(']');

        if (first == std::string::npos ||
            last == std::string::npos) {
            plate = "DELETE_THIS_PLATE";
            continue;
        }

        first += 1;
        plate = plate.substr(first, last - first);
        plate.erase(std::find_if(plate.rbegin(), plate.rend(), [](int ch) {
            return !std::isspace(ch);
            }).base(), plate.end());
    }

    Plates.erase(std::remove_if(
        Plates.begin(), Plates.end(),
        [](const auto& elem) -> bool { return elem == "DELETE_THIS_PLATE"; }
    ), Plates.end());

    Description = ini.GetValue("ID", "Description", "No description.");

    // [Main]
    LOAD_VAL("Main", "Enable", Enable);

    std::string mountPoint = ini.GetValue("Main", "MountPoint", "Vehicle");
    if (mountPoint == "Ped")
        MountPoint = EMount::Ped;
    else
        MountPoint = EMount::Vehicle;

    LOAD_VAL("Main", "MountIdPed", MountIdPed);
    LOAD_VAL("Main", "MountIdVehicle", MountIdVehicle);

    // [Look]
    LOAD_VAL("Look", "LookTime", Look.LookTime);
    LOAD_VAL("Look", "MouseLookTime", Look.MouseLookTime);
    LOAD_VAL("Look", "MouseCenterTimeout", Look.MouseCenterTimeout);
    LOAD_VAL("Look", "MouseSensitivity", Look.MouseSensitivity);

    // [Vehicle0-9]
    auto fnAddVehicle = [&](int i) {
        Vehicle.push_back(SCameraSettings{});
        LOAD_VAL_CAMERA("Vehicle" STR(i), Vehicle[i]);
        LOAD_VAL_MOVEMENT("Vehicle", Vehicle[i].Movement);
        LOAD_VAL_HORIZON("Vehicle", Vehicle[i].HorizonLock);
        LOAD_VAL_DOF("Vehicle", Vehicle[i].DoF);
    };

    Vehicle.clear();
    for (int i = 0; i < 10; ++i) {
        if (!ini.SectionExists("Vehicle" STR(i))) {
            break;
        }

        fnAddVehicle(i);
    }

    if (Vehicle.empty()) {
        LOG(WARN, "Empty Vehicle config section, creating default");
        fnAddVehicle(0);
    }

    // [Ped0-9]
    auto fnAddPed = [&](int i) {
        Ped.push_back(SCameraSettings{});
        LOAD_VAL_CAMERA("Ped" STR(i), Ped[i]);
        LOAD_VAL_MOVEMENT("Ped", Ped[i].Movement);
        LOAD_VAL_HORIZON("Ped", Ped[i].HorizonLock);
        LOAD_VAL_DOF("Ped", Ped[i].DoF);
    };

    Ped.clear();
    for (int i = 0; i < 10; ++i) {
        if (!ini.SectionExists("Ped" STR(i))) {
            break;
        }

        fnAddPed(i);
    }

    if (Ped.empty()) {
        LOG(WARN, "Empty Ped config section, creating default");
        fnAddPed(0);
    }
}

void CConfig::SaveSettings() {
    saveGeneral();
}

void CConfig::SaveSettings(CConfig* baseConfig, const std::string& customPath) {
    std::string oldPath = mFile;
    mFile = customPath;

    CConfig* oldConfig = mBaseConfig;
    mBaseConfig = baseConfig;

    saveGeneral();

    mBaseConfig = oldConfig;
    mFile = oldPath;
}

void CConfig::saveGeneral() {
    if (!mBaseConfig)
        LOG(FATAL, "CConfig::mBaseConfig not set. Skipping save!");

    CSimpleIniA ini;
    ini.SetUnicode();
    SI_Error result = ini.LoadFile(mFile.c_str());
    CHECK_LOG_SI_ERROR(result, std::format("load {}", mFile));

    // [ID]
    std::string modelNames = std::format("{}", StrUtil::Join(ModelNames, " ", "{}"));
    ini.SetValue("ID", "ModelName", modelNames.c_str());

    std::vector<std::string> fmtPlates;
    for (const auto& plate : Plates) {
        fmtPlates.push_back(std::format("[{}]", plate));
    }

    std::string plates = std::format("{}", StrUtil::Join(fmtPlates, ", ", "{}"));
    ini.SetValue("ID", "Plate", plates.c_str());

    ini.SetValue("ID", "Description", Description.c_str());

    // [Main]
    SAVE_VAL("Main", "Enable", Enable);

    if (MountPoint == EMount::Ped)
        SetValue(ini, "Main", "MountPoint", "Ped");
    else
        SetValue(ini, "Main", "MountPoint", "Vehicle");

    SAVE_VAL("Main", "MountIdPed", MountIdPed);
    SAVE_VAL("Main", "MountIdVehicle", MountIdVehicle);

    // [Look]
    SAVE_VAL("Look", "LookTime", Look.LookTime);
    SAVE_VAL("Look", "MouseLookTime", Look.MouseLookTime);
    SAVE_VAL("Look", "MouseCenterTimeout", Look.MouseCenterTimeout);
    SAVE_VAL("Look", "MouseSensitivity", Look.MouseSensitivity);

    // [Vehicle0-9]
    for (int i = 0; i < Vehicle.size(); ++i) {
        SAVE_VAL_CAMERA("Vehicle" STR(i), Vehicle[i]);
        SAVE_VAL_MOVEMENT("Vehicle", Vehicle[i].Movement);
        SAVE_VAL_HORIZON("Vehicle", Vehicle[i].HorizonLock);
        SAVE_VAL_DOF("Vehicle", Vehicle[i].DoF);
    }

    // [Ped0-9]
    for (int i = 0; i < Ped.size(); ++i) {
        SAVE_VAL_CAMERA("Ped" STR(i), Ped[i]);
        SAVE_VAL_MOVEMENT("Ped", Ped[i].Movement);
        SAVE_VAL_HORIZON("Ped", Ped[i].HorizonLock);
        SAVE_VAL_DOF("Ped", Ped[i].DoF);
    }

    result = ini.SaveFile(mFile.c_str());
    CHECK_LOG_SI_ERROR(result, std::format("save {}", mFile));
}

#pragma warning(pop)
