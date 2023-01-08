#include "FPVScript.hpp"

#include "Util/ScriptUtils.hpp"
#include "Util/Strings.hpp"

#include <inc/natives.h>
#include <format>

CFPVScript::CFPVScript(const std::shared_ptr<CScriptSettings>& settings, std::vector<CConfig>& configs)
    : mSettings(settings)
    , mConfigs(configs)
    , mDefaultConfig(configs[0])
    , mVehicle(0) {
}

void CFPVScript::Tick() {
    if (mActiveConfig) {
        update();
    }
}

void CFPVScript::Cancel() {
    // TODO
}

void CFPVScript::UpdateActiveConfig() {
    if (!ENTITY::DOES_ENTITY_EXIST(mVehicle)) {
        mActiveConfig = &mDefaultConfig;
        return;
    }

    Hash model = ENTITY::GET_ENTITY_MODEL(mVehicle);
    std::string plate = VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT(mVehicle);

    // First pass - match model and plate
    auto foundConfig = std::find_if(mConfigs.begin(), mConfigs.end(), [&](const CConfig& config) {
        bool modelMatch = config.ModelHash == model;
    bool plateMatch = StrUtil::Strcmpwi(config.Plate, plate);
    return modelMatch && plateMatch;
        });

    // second pass - match model with any plate
    if (foundConfig == mConfigs.end()) {
        foundConfig = std::find_if(mConfigs.begin(), mConfigs.end(), [&](const CConfig& config) {
            bool modelMatch = config.ModelHash == model;
        bool plateMatch = config.Plate.empty();
        return modelMatch && plateMatch;
            });
    }

    // third pass - use default
    if (foundConfig == mConfigs.end()) {
        mActiveConfig = &mDefaultConfig;
    }
    else {
        mActiveConfig = &*foundConfig;
    }
}


void CFPVScript::update() {
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);

    if (mVehicle != vehicle) {
        mVehicle = vehicle;
        UpdateActiveConfig();
    }
    mVehicle = vehicle;

    if (!Util::VehicleAvailable(vehicle, playerPed) ||
        !mActiveConfig ||
        !mSettings->Main.Enable) {
        // cancelCamera();
        return;
    }

    // TODO
}
