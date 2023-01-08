#pragma once
#include "Config.hpp"
#include "ScriptSettings.hpp"

#include <inc/types.h>
#include <memory>
#include <string>

class CFPVScript {
public:
    CFPVScript(const std::shared_ptr<CScriptSettings>& settings, std::vector<CConfig>& configs);
    ~CFPVScript() = default;
    void Tick();
    void Cancel();

    CConfig* ActiveConfig() {
        return mActiveConfig;
    }

    void UpdateActiveConfig();

    Vehicle GetVehicle() {
        return mVehicle;
    }

private:
    void update();

    const std::shared_ptr<CScriptSettings>& mSettings;
    std::vector<CConfig>& mConfigs;
    CConfig& mDefaultConfig;
    CConfig* mActiveConfig = nullptr;

    Vehicle mVehicle;
};
