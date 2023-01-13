#pragma once
#include "FPVScript.hpp"
#include "ScriptMenu.hpp"
#include "ShakeData.hpp"

namespace FPV {
    void ScriptMain();
    std::vector<CScriptMenu<CFPVScript>::CSubmenu> BuildMenu();

    CScriptSettings& GetSettings();
    CShakeData& GetShakeData();
    CFPVScript& GetScript();
    const std::vector<CConfig>& GetConfigs();

    uint32_t LoadConfigs();
    void SaveConfigs();
}
