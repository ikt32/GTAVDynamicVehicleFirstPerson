#include "FPVScript.hpp"

#include "Enums.hpp"
#include "Util/ScriptUtils.hpp"
#include "Util/Strings.hpp"

#include <inc/main.h>
#include <inc/natives.h>
#include <format>

CFPVScript::CFPVScript(const std::shared_ptr<CScriptSettings>& settings, std::vector<CConfig>& configs)
    : mSettings(settings)
    , mConfigs(configs)
    , mDefaultConfig(configs[0])
    , mVehicle(0)
    , mLookResetTimer(500) {
    // < VER_1_0_1290_1_STEAM
    if (getGameVersion() < 38) {
        mFpvCamOffsetXOffset = 0x428;
    }
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

void CFPVScript::Tick() {
    if (mActiveConfig) {
        update();
    }
    else {
        // mActiveConfig should never be nullptr
        Cancel();
        UpdateActiveConfig();
    }
}

void CFPVScript::Cancel() {
    if (CAM::DOES_CAM_EXIST(mHandle)) {
        CAM::RENDER_SCRIPT_CAMS(false, false, 0, true, false, 0);
        CAM::SET_CAM_ACTIVE(mHandle, false);
        CAM::DESTROY_CAM(mHandle, false);
        mHandle = -1;
        if (!mSettings->Debug.DisableRemoveHead) {
            hideHead(false);
        }
        HUD::UNLOCK_MINIMAP_ANGLE();
        GRAPHICS::SET_PARTICLE_FX_CAM_INSIDE_VEHICLE(false);
    }

    mRotation = {};
    mLookAcc = {};

    mInertiaDirectionLookAngle = 0.0f;
    mInertiaMove = {};
    mInertiaAccelPitchDeg = 0.0f;
    mDynamicPitchDeg = 0.0f;

    mAverageAccel = 0.0f;
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
        Cancel();
        return;
    }

    // TODO
}

void CFPVScript::hideHead(bool remove) {
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    if (Dismemberment::Available()) {
        if (remove) {
            Dismemberment::AddBoneDraw(playerPed, 0x796E, -1);
            mHeadRemoved = true;
        }
        else {
            Dismemberment::RemoveBoneDraw(playerPed);
            mHeadRemoved = false;
        }
    }

    if (!mSettings->Debug.DisableRemoveProps) {
        if (remove) {
            savedHeadProp = PED::GET_PED_PROP_INDEX(playerPed, static_cast<int>(ePedPropPosition::AnchorHead));
            savedHeadPropTx = PED::GET_PED_PROP_TEXTURE_INDEX(playerPed, static_cast<int>(ePedPropPosition::AnchorHead));

            savedEyesProp = PED::GET_PED_PROP_INDEX(playerPed, static_cast<int>(ePedPropPosition::AnchorEyes));
            savedEyesPropTx = PED::GET_PED_PROP_TEXTURE_INDEX(playerPed, static_cast<int>(ePedPropPosition::AnchorEyes));

            PED::CLEAR_PED_PROP(playerPed, static_cast<int>(ePedPropPosition::AnchorHead));
            PED::CLEAR_PED_PROP(playerPed, static_cast<int>(ePedPropPosition::AnchorEyes));
        }
        else {
            if (savedHeadProp != -1)
                PED::SET_PED_PROP_INDEX(playerPed, static_cast<int>(ePedPropPosition::AnchorHead), savedHeadProp, savedHeadPropTx, true);
            if (savedEyesProp != -1)
                PED::SET_PED_PROP_INDEX(playerPed, static_cast<int>(ePedPropPosition::AnchorEyes), savedEyesProp, savedEyesPropTx, true);
        }
    }
}