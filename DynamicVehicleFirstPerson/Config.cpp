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

void SetValue(CSimpleIniA & ini, const char* section, const char* key, CConfig::EMountPoint val) {
    ini.SetLongValue(section, key, static_cast<int>(val));
}

CConfig::EMountPoint GetValue(CSimpleIniA& ini, const char* section, const char* key, CConfig::EMountPoint val) {
    int outVal = ini.GetLongValue(section, key, static_cast<int>(val));
    if (outVal != 0 || outVal != 1) {
        return CConfig::EMountPoint::Vehicle;
    }
    return static_cast<CConfig::EMountPoint>(outVal);
}

#define SAVE_VAL_MOVEMENT(section, source) { \
    SAVE_VAL(section, "Follow",            ##source.Follow); \
    SAVE_VAL(section, "RotationDirectionMult", ##source.RotationDirectionMult); \
    SAVE_VAL(section, "RotationRotationMult",  ##source.RotationRotationMult); \
    SAVE_VAL(section, "RotationMaxAngle",      ##source.RotationMaxAngle); \
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
    LOAD_VAL(section, "Follow",            ##source.Follow); \
    LOAD_VAL(section, "RotationDirectionMult", ##source.RotationDirectionMult); \
    LOAD_VAL(section, "RotationRotationMult",  ##source.RotationRotationMult); \
    LOAD_VAL(section, "RotationMaxAngle",      ##source.RotationMaxAngle); \
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
    SAVE_VAL(section, "Lock",        ##source.Lock); \
    SAVE_VAL(section, "PitchMode",   ##source.PitchMode); \
    SAVE_VAL(section, "CenterSpeed", ##source.CenterSpeed); \
    SAVE_VAL(section, "PitchLim",    ##source.PitchLim); \
    SAVE_VAL(section, "RollLim",     ##source.RollLim); \
}

#define LOAD_VAL_HORIZON(section, source) { \
    LOAD_VAL(section, "Lock",        ##source.Lock); \
    LOAD_VAL(section, "PitchMode",   ##source.PitchMode); \
    LOAD_VAL(section, "CenterSpeed", ##source.CenterSpeed); \
    LOAD_VAL(section, "PitchLim",    ##source.PitchLim); \
    LOAD_VAL(section, "RollLim",     ##source.RollLim); \
}

#define SAVE_VAL_DOF(section, source) { \
    SAVE_VAL(section, "Enable",                   ##source.Enable); \
    SAVE_VAL(section, "TargetSpeedMinDoF",        ##source.TargetSpeedMinDoF); \
    SAVE_VAL(section, "TargetSpeedMaxDoF",        ##source.TargetSpeedMaxDoF); \
    SAVE_VAL(section, "TargetAccelMinDoF",        ##source.TargetAccelMinDoF); \
    SAVE_VAL(section, "TargetAccelMaxDoF",        ##source.TargetAccelMaxDoF); \
    SAVE_VAL(section, "TargetAccelMinDoFMod",     ##source.TargetAccelMinDoFMod); \
    SAVE_VAL(section, "TargetAccelMaxDoFMod",     ##source.TargetAccelMaxDoFMod); \
    SAVE_VAL(section, "NearOutFocusMinSpeedDist", ##source.NearOutFocusMinSpeedDist); \
    SAVE_VAL(section, "NearOutFocusMaxSpeedDist", ##source.NearOutFocusMaxSpeedDist); \
    SAVE_VAL(section, "NearInFocusMinSpeedDist",  ##source.NearInFocusMinSpeedDist); \
    SAVE_VAL(section, "NearInFocusMaxSpeedDist",  ##source.NearInFocusMaxSpeedDist); \
    SAVE_VAL(section, "FarInFocusMinSpeedDist",   ##source.FarInFocusMinSpeedDist); \
    SAVE_VAL(section, "FarInFocusMaxSpeedDist",   ##source.FarInFocusMaxSpeedDist); \
    SAVE_VAL(section, "FarOutFocusMinSpeedDist",  ##source.FarOutFocusMinSpeedDist); \
    SAVE_VAL(section, "FarOutFocusMaxSpeedDist",  ##source.FarOutFocusMaxSpeedDist); \
}

#define LOAD_VAL_DOF(section, source) { \
    LOAD_VAL(section, "Enable",                   ##source.Enable); \
    LOAD_VAL(section, "TargetSpeedMinDoF",        ##source.TargetSpeedMinDoF); \
    LOAD_VAL(section, "TargetSpeedMaxDoF",        ##source.TargetSpeedMaxDoF); \
    LOAD_VAL(section, "TargetAccelMinDoF",        ##source.TargetAccelMinDoF); \
    LOAD_VAL(section, "TargetAccelMaxDoF",        ##source.TargetAccelMaxDoF); \
    LOAD_VAL(section, "TargetAccelMinDoFMod",     ##source.TargetAccelMinDoFMod); \
    LOAD_VAL(section, "TargetAccelMaxDoFMod",     ##source.TargetAccelMaxDoFMod); \
    LOAD_VAL(section, "NearOutFocusMinSpeedDist", ##source.NearOutFocusMinSpeedDist); \
    LOAD_VAL(section, "NearOutFocusMaxSpeedDist", ##source.NearOutFocusMaxSpeedDist); \
    LOAD_VAL(section, "NearInFocusMinSpeedDist",  ##source.NearInFocusMinSpeedDist); \
    LOAD_VAL(section, "NearInFocusMaxSpeedDist",  ##source.NearInFocusMaxSpeedDist); \
    LOAD_VAL(section, "FarInFocusMinSpeedDist",   ##source.FarInFocusMinSpeedDist); \
    LOAD_VAL(section, "FarInFocusMaxSpeedDist",   ##source.FarInFocusMaxSpeedDist); \
    LOAD_VAL(section, "FarOutFocusMinSpeedDist",  ##source.FarOutFocusMinSpeedDist); \
    LOAD_VAL(section, "FarOutFocusMaxSpeedDist",  ##source.FarOutFocusMaxSpeedDist); \
}

#define SAVE_VAL_CAMERA(section, source) { \
    SAVE_VAL(section, "MountPoint",    ##source.MountPoint); \
    SAVE_VAL(section, "FOV",           ##source.FOV); \
    SAVE_VAL(section, "OffsetHeight",  ##source.OffsetHeight); \
    SAVE_VAL(section, "OffsetForward", ##source.OffsetForward); \
    SAVE_VAL(section, "OffsetSide",    ##source.OffsetSide); \
    SAVE_VAL(section, "Pitch",         ##source.Pitch); \
}

#define LOAD_VAL_CAMERA(section, source) { \
    LOAD_VAL(section, "MountPoint",    ##source.MountPoint); \
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
    LOAD_VAL("Main", "CamIndex", config.CamIndex);

    // [Look]
    LOAD_VAL("Look", "LookTime", config.Look.LookTime);
    LOAD_VAL("Look", "MouseLookTime", config.Look.MouseLookTime);
    LOAD_VAL("Look", "MouseCenterTimeout", config.Look.MouseCenterTimeout);
    LOAD_VAL("Look", "MouseSensitivity", config.Look.MouseSensitivity);

    // [Mount0-9]
    auto fnAddMount = [&](int i) {
        config.Mount.push_back(SCameraSettings{});
        LOAD_VAL_CAMERA(    "Mount" STR(i),             config.Mount[i]);
        LOAD_VAL_MOVEMENT(  "Mount" STR(i) ".Movement", config.Mount[i].Movement);
        LOAD_VAL_HORIZON(   "Mount" STR(i) ".Horizon",  config.Mount[i].HorizonLock);
        LOAD_VAL_DOF(       "Mount" STR(i) ".DoF",      config.Mount[i].DoF);
    };

    config.Mount.clear();
    for (int i = 0; i < 10; ++i) {
        if (!ini.SectionExists("Mount" STR(i))) {
            break;
        }

        fnAddMount(i);
    }

    if (config.Mount.empty()) {
        LOG(WARN, "[Config] Empty Mount config section, creating default");
        fnAddMount(0);
    }

    if (config.CamIndex >= config.Mount.size()) {
        LOG(WARN, "[Config] CamIndex out of range ({}), reset to {}",
            config.CamIndex,
            config.Mount.size() - 1);
        config.CamIndex = static_cast<int>(config.Mount.size()) - 1;
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

        std::string modelName = ASCache::GetCachedModelName(ModelHash);
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
    SAVE_VAL("Main", "CamIndex", CamIndex);

    // [Look]
    SAVE_VAL("Look", "LookTime", Look.LookTime);
    SAVE_VAL("Look", "MouseLookTime", Look.MouseLookTime);
    SAVE_VAL("Look", "MouseCenterTimeout", Look.MouseCenterTimeout);
    SAVE_VAL("Look", "MouseSensitivity", Look.MouseSensitivity);

    // [Mount0-9]
    for (int i = 0; i < Mount.size(); ++i) {
        SAVE_VAL_CAMERA(    "Mount" STR(i),             Mount[i]);
        SAVE_VAL_MOVEMENT(  "Mount" STR(i) ".Movement", Mount[i].Movement);
        SAVE_VAL_HORIZON(   "Mount" STR(i) ".Horizon",  Mount[i].HorizonLock);
        SAVE_VAL_DOF(       "Mount" STR(i) ".DoF",      Mount[i].DoF);
    }

    result = ini.SaveFile(configFile.c_str());
    CHECK_LOG_SI_ERROR(result, "save", configFile.string());
    if (result < 0)
        return false;
    return true;
}
