#include "Config.hpp"

#include "Constants.hpp"
#include "SettingsCommon.hpp"
#include "Util/AddonSpawnerCache.hpp"
#include "Util/Logger.hpp"
#include "Util/Paths.hpp"
#include "Util/Strings.hpp"

#include <simpleini/SimpleIni.h>
#include <filesystem>
#include <cctype>

#define CHECK_LOG_SI_ERROR(result, operation, file) \
    if ((result) < 0) { \
        LOG(ERROR, "[Config] {} Failed to {}, SI_Error [{}]", \
        file, operation, result); \
    }

#define SAVE_VAL(section, key, option)\
    {\
        SetValue(ini, section, key, option);\
    }

#define LOAD_VAL(section, key, option)\
    {\
        option = GetValue(ini, section, key, option);\
    }

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

CConfig CConfig::Read(const std::string& configFile) {
    CConfig config{};

    CSimpleIniA ini;
    ini.SetUnicode();
    SI_Error result = ini.LoadFile(configFile.c_str());
    CHECK_LOG_SI_ERROR(result, "load", configFile.c_str());

    config.Name = std::filesystem::path(configFile).stem().string();

    // [ID]
    std::string modelNamesAll = ini.GetValue("ID", "Models", "");
    std::string modelHashStr = ini.GetValue("ID", "ModelHash", "");
    std::string modelName = ini.GetValue("ID", "ModelName", "");

    if (modelHashStr.empty() && modelName.empty()) {
        // This is a no-vehicle config. Nothing to be done.
    }
    else if (modelHashStr.empty()) {
        // This config only has a model name.
        config.ModelHash = StrUtil::Joaat(modelName.c_str());
        config.ModelName = modelName;
    }
    else {
        // This config only has a hash.
        Hash modelHash = 0;
        int found = sscanf_s(modelHashStr.c_str(), "%X", &modelHash);

        if (found == 1) {
            config.ModelHash = modelHash;

            auto& asCache = ASCache::Get();
            auto it = asCache.find(modelHash);
            std::string modelName = it == asCache.end() ? std::string() : it->second;
            config.ModelName = modelName;
        }
    }

    config.Plate = ini.GetValue("ID", "Plate", "");

    // [Main]
    LOAD_VAL("Main", "Enable", config.Enable);

    std::string mountPoint = ini.GetValue("Main", "MountPoint", "Vehicle");
    if (mountPoint == "Ped")
        config.MountPoint = EMount::Ped;
    else
        config.MountPoint = EMount::Vehicle;

    LOAD_VAL("Main", "MountIdPed", config.MountIdPed);
    LOAD_VAL("Main", "MountIdVehicle", config.MountIdVehicle);

    // [Look]
    LOAD_VAL("Look", "LookTime", config.Look.LookTime);
    LOAD_VAL("Look", "MouseLookTime", config.Look.MouseLookTime);
    LOAD_VAL("Look", "MouseCenterTimeout", config.Look.MouseCenterTimeout);
    LOAD_VAL("Look", "MouseSensitivity", config.Look.MouseSensitivity);

    // [Vehicle0-9]
    auto fnAddVehicle = [&](int i) {
        config.Vehicle.push_back(SCameraSettings{});
        LOAD_VAL_CAMERA("Vehicle" STR(i), config.Vehicle[i]);
        LOAD_VAL_MOVEMENT("Vehicle", config.Vehicle[i].Movement);
        LOAD_VAL_HORIZON("Vehicle", config.Vehicle[i].HorizonLock);
        LOAD_VAL_DOF("Vehicle", config.Vehicle[i].DoF);
    };

    config.Vehicle.clear();
    for (int i = 0; i < 10; ++i) {
        if (!ini.SectionExists("Vehicle" STR(i))) {
            break;
        }

        fnAddVehicle(i);
    }

    if (config.Vehicle.empty()) {
        LOG(WARN, "Empty Vehicle config section, creating default");
        fnAddVehicle(0);
    }

    // [Ped0-9]
    auto fnAddPed = [&](int i) {
        config.Ped.push_back(SCameraSettings{});
        LOAD_VAL_CAMERA("Ped" STR(i), config.Ped[i]);
        LOAD_VAL_MOVEMENT("Ped", config.Ped[i].Movement);
        LOAD_VAL_HORIZON("Ped", config.Ped[i].HorizonLock);
        LOAD_VAL_DOF("Ped", config.Ped[i].DoF);
    };

    config.Ped.clear();
    for (int i = 0; i < 10; ++i) {
        if (!ini.SectionExists("Ped" STR(i))) {
            break;
        }

        fnAddPed(i);
    }

    if (config.Ped.empty()) {
        LOG(WARN, "Empty Ped config section, creating default");
        fnAddPed(0);
    }

    return config;
}

void CConfig::Write(ESaveType saveType) {
    Write(Name, 0, std::string(), saveType);
}

bool CConfig::Write(const std::string& newName, Hash model, std::string plate, ESaveType saveType) {
    const auto configsPath = Paths::GetModPath() / "Configs";
    const auto configFile = configsPath / std::format("{}.ini", newName);

    CSimpleIniA ini;
    ini.SetUnicode();
    ini.SetMultiLine(true);

    // This here MAY fail on first save, in which case, it can be ignored.
    // _Not_ having this just nukes the entire file, including any comments.
    SI_Error result = ini.LoadFile(configFile.c_str());
    if (result < 0) {
        LOG(WARN, "[Config] {} Failed to load, SI_Error [{}]. (No problem if no file exists yet)",
            configFile.string(), result);
    }

    // [ID]
    if (saveType != ESaveType::GenericNone) {
        if (model != 0) {
            ModelHash = model;
        }

        ini.SetValue("ID", "ModelHash", std::format("{:X}", ModelHash).c_str());

        auto& asCache = ASCache::Get();
        auto it = asCache.find(ModelHash);
        std::string modelName = it == asCache.end() ? std::string() : it->second;
        if (!modelName.empty()) {
            ModelName = modelName;
            ini.SetValue("ID", "ModelName", modelName.c_str());
        }

        if (saveType == ESaveType::Specific) {
            Plate = plate;
            ini.SetValue("ID", "Plate", plate.c_str());
        }
    }

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

    result = ini.SaveFile(configFile.c_str());
    CHECK_LOG_SI_ERROR(result, "save", configFile.string());
    if (result < 0)
        return false;
    return true;
}
