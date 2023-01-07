#pragma once
#include "FPVScript.hpp"
#include "ScriptMenu.hpp"

namespace FPV {
    void ScriptMain();
    std::vector<CScriptMenu<CFPVScript>::CSubmenu> BuildMenu();

    CScriptSettings& GetSettings();
    CFPVScript& GetScript();
    const std::vector<CConfig>& GetConfigs();

    uint32_t LoadConfigs();
    void SaveConfigs();
}
