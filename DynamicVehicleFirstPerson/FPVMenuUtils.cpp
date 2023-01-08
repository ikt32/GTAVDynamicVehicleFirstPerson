#include "FPVMenuUtils.hpp"

#include "Script.hpp"

#include "Util/AddonSpawnerCache.hpp"
#include "Util/Paths.hpp"
#include "Util/UI.hpp"
#include "Util/Strings.hpp"

#include <inc/natives.h>

#include <string>

void FPV::CreateConfig(CConfig& config, Vehicle vehicle) {
    const auto vehConfigDir = Paths::GetModPath() / "Configs";

    // Pre-fill with actual model name, if Add-on Spawner is present.
    const std::string modelName = ASCache::GetCachedModelName(ENTITY::GET_ENTITY_MODEL(vehicle));

    UI::ShowHelpText("Enter configuration name.");
    std::string cfgName = FPV::GetKbEntryString(modelName);
    if (cfgName.empty()) {
        UI::Notify("No name entered, cancelled configuration save.");
        return;
    }

    UI::ShowHelpText("Enter '1' for a generic model configuration.\n"
        "Enter '2' for a model and plate matched configuration.");
    std::string configMode = FPV::GetKbEntryString("");

    CConfig::ESaveType saveType;
    if (configMode == "1") {
        saveType = CConfig::ESaveType::GenericModel;
    }
    else if (configMode == "2") {
        saveType = CConfig::ESaveType::Specific;
    }
    else {
        UI::ShowHelpText("No supported type entered, cancelled configuration save.");
        return;
    }

    auto model = ENTITY::GET_ENTITY_MODEL(vehicle);
    std::string plate = VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT(vehicle);

    if (config.Write(cfgName, model, plate, saveType))
        UI::Notify("New configuration saved.", true);
    else
        UI::Notify("~r~An error occurred~s~, failed to save new configuration.\n"
            "Check the log file for further details.", true);
    FPV::LoadConfigs();
}

std::string FPV::MountName(CConfig::EMountPoint mount) {
    switch (mount) {
        case CConfig::EMountPoint::Vehicle: return "Vehicle";
        case CConfig::EMountPoint::Ped: return "Ped";
    }
    return "Invalid";
}

std::vector<std::string> FPV::FormatConfigInfo(const CConfig& cfg) {
    std::string modelName = cfg.ModelName.empty() ?
        "No model" : cfg.ModelName;
    std::string plate = cfg.Plate.empty() ?
        "No plate" : cfg.Plate;
    return {
        std::format("~h~{}", cfg.Name),
        std::format("Model/Plate: {}/[{}]", modelName, plate),
        std::format("Cameras: {}", cfg.Mount.size())
    };
}

std::vector<std::string> FPV::FormatCameraInfo(const CConfig& cfg) {
    std::string horizonLock;
    if (cfg.Mount[cfg.CamIndex].HorizonLock.Lock) {
        switch (cfg.Mount[cfg.CamIndex].HorizonLock.PitchMode) {
            case 0: horizonLock = "Yes: Full"; break;
            case 1: horizonLock = "Yes: Roll only"; break;
            case 2: horizonLock = "Yes: Dynamic"; break;
            default: horizonLock = std::format("Index {} out of range", cfg.CamIndex);
        }
    }
    else {
        horizonLock = "No";
    }

    return {
        std::format("ID {} (max {})", cfg.CamIndex, cfg.Mount.size()),
        std::format("FOV: {:.1f}", cfg.Mount[cfg.CamIndex].FOV),
        std::format("Horizon lock: {}", horizonLock),
        std::format("Inertia: {}", cfg.Mount[cfg.CamIndex].Movement.Follow ? "Yes" : "No"),
        std::format("DoF: {}", cfg.Mount[cfg.CamIndex].DoF.Enable ? "Yes" : "No")
    };
}

std::string FPV::GetKbEntryString(const std::string& existingString) {
    std::string val;
    UI::Notify("Enter value");
    MISC::DISPLAY_ONSCREEN_KEYBOARD(LOCALIZATION::GET_CURRENT_LANGUAGE() == 0, "FMMC_KEY_TIP8", "",
        existingString.c_str(), "", "", "", 64);
    while (MISC::UPDATE_ONSCREEN_KEYBOARD() == 0) {
        WAIT(0);
    }
    if (!MISC::GET_ONSCREEN_KEYBOARD_RESULT()) {
        UI::Notify("Cancelled value entry");
        return {};
    }

    std::string enteredVal = MISC::GET_ONSCREEN_KEYBOARD_RESULT();
    if (enteredVal.empty()) {
        UI::Notify("Cancelled value entry");
        return {};
    }

    return enteredVal;
}

bool FPV::GetKbEntryInt(int& val) {
    UI::Notify("Enter value");
    MISC::DISPLAY_ONSCREEN_KEYBOARD(LOCALIZATION::GET_CURRENT_LANGUAGE() == 0, "FMMC_KEY_TIP8", "",
        std::format("{}", val).c_str(), "", "", "", 64);
    while (MISC::UPDATE_ONSCREEN_KEYBOARD() == 0) {
        WAIT(0);
    }
    if (!MISC::GET_ONSCREEN_KEYBOARD_RESULT()) {
        UI::Notify("Cancelled value entry");
        return false;
    }

    std::string intStr = MISC::GET_ONSCREEN_KEYBOARD_RESULT();
    if (intStr.empty()) {
        UI::Notify("Cancelled value entry");
        return false;
    }

    char* pEnd;
    int parsedValue = strtol(intStr.c_str(), &pEnd, 10);

    if (parsedValue == 0 && *pEnd != 0) {
        UI::Notify("Failed to parse entry.");
        return false;
    }

    val = parsedValue;
    return true;
}

bool FPV::GetKbEntryFloat(float& val) {
    UI::Notify("Enter value");
    MISC::DISPLAY_ONSCREEN_KEYBOARD(LOCALIZATION::GET_CURRENT_LANGUAGE() == 0, "FMMC_KEY_TIP8", "",
        std::format("{:f}", val).c_str(), "", "", "", 64);
    while (MISC::UPDATE_ONSCREEN_KEYBOARD() == 0) {
        WAIT(0);
    }
    if (!MISC::GET_ONSCREEN_KEYBOARD_RESULT()) {
        UI::Notify("Cancelled value entry");
        return false;
    }

    std::string floatStr = MISC::GET_ONSCREEN_KEYBOARD_RESULT();
    if (floatStr.empty()) {
        UI::Notify("Cancelled value entry");
        return false;
    }

    char* pEnd;
    float parsedValue = strtof(floatStr.c_str(), &pEnd);

    if (parsedValue == 0.0f && *pEnd != 0) {
        UI::Notify("Failed to parse entry.");
        return false;
    }

    val = parsedValue;
    return true;
}
