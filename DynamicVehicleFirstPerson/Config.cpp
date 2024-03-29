#include "Config.hpp"

#include "SettingsCommon.hpp"
#include "Util/AddonSpawnerCache.hpp"
#include "Util/Logger.hpp"
#include "Util/Paths.hpp"
#include "Util/Strings.hpp"

#include <simpleini/SimpleIni.h>
#include <filesystem>
#include <cctype>

using std::to_underlying;

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

void SetValue(CSimpleIniA & ini, const std::string& section, const char* key, CConfig::EMountPoint val) {
    ini.SetLongValue(section.c_str(), key, to_underlying(val));
}

CConfig::EMountPoint GetValue(CSimpleIniA& ini, const std::string& section, const char* key, CConfig::EMountPoint val) {
    int outVal = ini.GetLongValue(section.c_str(), key, to_underlying(val));
    if (outVal != 0 && outVal != 1) {
        return CConfig::EMountPoint::Vehicle;
    }
    return static_cast<CConfig::EMountPoint>(outVal);
}

#define SAVE_VAL_LEAN(section, source) { \
    SAVE_VAL(section, "CenterDist",  ##source.CenterDist); \
    SAVE_VAL(section, "ForwardDist", ##source.ForwardDist); \
    SAVE_VAL(section, "UpDist",      ##source.UpDist); \
}

#define LOAD_VAL_LEAN(section, source) { \
    LOAD_VAL(section, "CenterDist",  ##source.CenterDist); \
    LOAD_VAL(section, "ForwardDist", ##source.ForwardDist); \
    LOAD_VAL(section, "UpDist",      ##source.UpDist); \
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
    SAVE_VAL(section, "ShakeSpeed",        ##source.ShakeSpeed); \
    SAVE_VAL(section, "ShakeTerrain",      ##source.ShakeTerrain); \
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
    LOAD_VAL(section, "ShakeSpeed",        ##source.ShakeSpeed); \
    LOAD_VAL(section, "ShakeTerrain",      ##source.ShakeTerrain); \
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
    SAVE_VAL(section, "Order",         ##source.Order); \
    SAVE_VAL(section, "MountPoint",    ##source.MountPoint); \
    SAVE_VAL(section, "FOV",           ##source.FOV); \
    SAVE_VAL(section, "OffsetHeight",  ##source.OffsetHeight); \
    SAVE_VAL(section, "OffsetForward", ##source.OffsetForward); \
    SAVE_VAL(section, "OffsetSide",    ##source.OffsetSide); \
    SAVE_VAL(section, "Pitch",         ##source.Pitch); \
}

#define LOAD_VAL_CAMERA(section, source) { \
    LOAD_VAL(section, "Order",         ##source.Order); \
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
    if (result < 0) {
        return {};
    }

    config.Name = std::filesystem::path(configFile).stem().string();
    LOG(DEBUG, "[Config] Reading {}", config.Name);

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

    // [Mount<Name>]
    auto fnAddMount = [&](const std::string& name) {
        config.Mount.push_back(SCameraSettings{
            .Name = name
        });
        auto& mount = config.Mount.back();
        LOAD_VAL_CAMERA(  std::format("Mount{}", name),          mount);
        LOAD_VAL_LEAN(    std::format("Mount{}.Lean", name),     mount.Lean);
        LOAD_VAL_MOVEMENT(std::format("Mount{}.Movement", name), mount.Movement);
        LOAD_VAL_HORIZON( std::format("Mount{}.Horizon", name),  mount.HorizonLock);
        LOAD_VAL_DOF(     std::format("Mount{}.DoF", name),      mount.DoF);
    };

    std::list<CSimpleIniA::Entry> allSections;
    ini.GetAllSections(allSections);

    std::vector<std::string> mountNames;
    for (const auto& section : allSections) {
        std::string sectionName = section.pItem;
        if (sectionName.starts_with("Mount") &&
            !sectionName.ends_with(".Lean") &&
            !sectionName.ends_with(".Movement") &&
            !sectionName.ends_with(".Horizon") &&
            !sectionName.ends_with(".DoF")) {

            std::string configName = sectionName.substr(strlen("Mount"));
            if (std::find(mountNames.begin(), mountNames.end(), configName) != mountNames.end()) {
                LOG(ERROR, "[Config] Section name '{}' is duplicated. Skipping config...");
                return {};
            }
            mountNames.push_back(configName);
        }
    }

    config.Mount.clear();
    for (const auto& mountName : mountNames) {
        fnAddMount(mountName);
    }

    if (config.Mount.size() > 1) {
        std::sort(config.Mount.begin(), config.Mount.end(),
            [](const SCameraSettings& cam1, const SCameraSettings& cam2)->bool {
                return cam1.Order < cam2.Order;
            });

        auto duplicate = std::adjacent_find(config.Mount.begin(), config.Mount.end(),
            [](const auto& mount1, const auto& mount2) {
                return mount1.Order == mount2.Order;
            }
        );
        while (config.Mount.size() > 1 && duplicate != config.Mount.end()) {
            LOG(ERROR, "[Config] Duplicate Order found in Mount '{}': {}, removed",
                duplicate->Name, duplicate->Order);
            config.Mount.erase(duplicate);
        }
    }

    if (config.Mount.empty()) {
        LOG(WARN, "[Config] Empty Mount config section, creating default");
        fnAddMount("Default");
    }

    if (config.CamIndex >= config.Mount.size()) {
        LOG(WARN, "[Config] CamIndex out of range ({}), reset to {}",
            config.CamIndex,
            config.Mount.size() - 1);
        config.CamIndex = static_cast<int>(config.Mount.size()) - 1;
    }

    LOG(DEBUG, "[Config] Loaded {}", config.Name);
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

    LOG(DEBUG, "[Config] Saving {}", Name);

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

    if (Mount.empty()) {
        LOG(DEBUG, "Saving config without camera, inserting default.");
        Mount.push_back(SCameraSettings{ .Name = "Default" });
    }

    // [Mount<Name>]
    for (const auto& mount : Mount) {
        const auto& name = mount.Name;
        SAVE_VAL_CAMERA(  std::format("Mount{}", name),          mount);
        SAVE_VAL_LEAN(    std::format("Mount{}.Lean", name),     mount.Lean);
        SAVE_VAL_MOVEMENT(std::format("Mount{}.Movement", name), mount.Movement);
        SAVE_VAL_HORIZON( std::format("Mount{}.Horizon", name),  mount.HorizonLock);
        SAVE_VAL_DOF(     std::format("Mount{}.DoF", name),      mount.DoF);
    }

    result = ini.SaveFile(configFile.c_str());
    CHECK_LOG_SI_ERROR(result, "save", configFile.string());
    if (result < 0) {
        LOG(ERROR, "[Config] Failed to save {}", Name);
        return false;
    }
    LOG(DEBUG, "[Config] Saved {}", Name);
    return true;
}

void CConfig::DeleteCamera(const std::string& camToDelete) {
    const auto configsPath = Paths::GetModPath() / "Configs";
    const auto configFile = configsPath / std::format("{}.ini", Name);

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

    std::erase_if(Mount, [camToDelete](const auto& mount) { return mount.Name == camToDelete; });

    ini.Delete(std::format("Mount{}", camToDelete).c_str(), nullptr, true);
    ini.Delete(std::format("Mount{}.Lean", camToDelete).c_str(), nullptr, true);
    ini.Delete(std::format("Mount{}.Movement", camToDelete).c_str(), nullptr, true);
    ini.Delete(std::format("Mount{}.Horizon", camToDelete).c_str(), nullptr, true);
    ini.Delete(std::format("Mount{}.DoF", camToDelete).c_str(), nullptr, true);

    result = ini.SaveFile(configFile.c_str());
    CHECK_LOG_SI_ERROR(result, "save", configFile.string());
}
